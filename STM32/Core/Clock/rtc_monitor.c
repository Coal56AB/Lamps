#include "rtc_monitor.h"

#include "main.h"
#include "rtc.h"

#define RTC_SECONDS_PER_DAY             86400U
#define RTC_MONITOR_WINDOW_SECONDS      16U
#define RTC_MONITOR_ALLOWED_LAG_SECONDS 2U
#define RTC_MONITOR_BAD_WINDOWS         2U
#define RTC_LSE_OFF_TIMEOUT_MS          500U
#define RTC_LSE_START_TIMEOUT_SECONDS   12U
#define RTC_LSE_SYNC_TIMEOUT_SECONDS    1U
#define RTC_LSE_MAX_START_ATTEMPTS      3U
#define RTC_LSE_RETRY_SECONDS           60U

typedef enum
{
    RTC_PHASE_STOPPED = 0,
    RTC_PHASE_MONITORING,
    RTC_PHASE_WAIT_LSE_OFF,
    RTC_PHASE_WAIT_LSE_READY,
    RTC_PHASE_WAIT_LSE_SYNC,
    RTC_PHASE_RETRY_DELAY
} RTC_MonitorPhase;

static uint8_t initialized;
static uint8_t bad_windows;
static uint8_t start_attempts;
static uint32_t core_clock_hz;
static uint32_t last_cycle_counter;
static uint32_t sample_rtc_seconds;
static uint32_t reference_rtc_seconds;
static uint64_t total_cycles;
static uint64_t sample_cycles;
static uint64_t reference_cycles;
static uint64_t phase_started_cycles;
static RTC_MonitorPhase phase = RTC_PHASE_STOPPED;
static RTC_MonitorStatus status = RTC_MONITOR_LSE_FAILED;
static uint32_t recovery_count;
static uint32_t failure_count;

static void UpdateCycleReference(void)
{
    uint32_t now = DWT->CYCCNT;
    total_cycles += (uint32_t)(now - last_cycle_counter);
    last_cycle_counter = now;
}

static uint32_t ReadRtcCounter(void)
{
    uint16_t high_before;
    uint16_t high_after;
    uint16_t low;

    high_before = (uint16_t)(RTC->CNTH & RTC_CNTH_RTC_CNT);
    low = (uint16_t)(RTC->CNTL & RTC_CNTL_RTC_CNT);
    high_after = (uint16_t)(RTC->CNTH & RTC_CNTH_RTC_CNT);

    if (high_before != high_after)
    {
        low = (uint16_t)(RTC->CNTL & RTC_CNTL_RTC_CNT);
    }

    return ((((uint32_t)high_after << 16U) | low) % RTC_SECONDS_PER_DAY);
}

static uint32_t RtcElapsed(uint32_t from, uint32_t to)
{
    return (to + RTC_SECONDS_PER_DAY - from) % RTC_SECONDS_PER_DAY;
}

static uint64_t CyclesForSeconds(uint32_t seconds)
{
    return (uint64_t)core_clock_hz * seconds;
}

static uint64_t CyclesForMilliseconds(uint32_t milliseconds)
{
    return ((uint64_t)core_clock_hz * milliseconds) / 1000U;
}

static uint32_t ExpectedRtcSeconds(void)
{
    uint64_t elapsed_cycles = total_cycles - reference_cycles;
    uint32_t elapsed_seconds = (uint32_t)(elapsed_cycles / core_clock_hz);

    return (reference_rtc_seconds + elapsed_seconds) % RTC_SECONDS_PER_DAY;
}

static void RebaseReferences(uint32_t rtc_seconds)
{
    reference_rtc_seconds = rtc_seconds;
    reference_cycles = total_cycles;
    sample_rtc_seconds = rtc_seconds;
    sample_cycles = total_cycles;
    bad_windows = 0U;
}

static void StartLse(void)
{
    __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);
    phase_started_cycles = total_cycles;
    start_attempts++;
    phase = RTC_PHASE_WAIT_LSE_READY;
}

static void BeginRecovery(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    __HAL_RCC_LSE_CONFIG(RCC_LSE_OFF);
    phase_started_cycles = total_cycles;
    start_attempts = 0U;
    phase = RTC_PHASE_WAIT_LSE_OFF;
    status = RTC_MONITOR_RECOVERING;
}

static void RetryLseStartOrDelay(void)
{
    if (start_attempts < RTC_LSE_MAX_START_ATTEMPTS)
    {
        __HAL_RCC_LSE_CONFIG(RCC_LSE_OFF);
        phase_started_cycles = total_cycles;
        phase = RTC_PHASE_WAIT_LSE_OFF;
    }
    else
    {
        failure_count++;
        status = RTC_MONITOR_LSE_FAILED;
        phase_started_cycles = total_cycles;
        phase = RTC_PHASE_RETRY_DELAY;
    }
}

static void RestoreExpectedTime(void)
{
    RTC_TimeTypeDef rtc_time = {0};
    uint32_t expected = ExpectedRtcSeconds();

    rtc_time.Hours = (uint8_t)(expected / 3600U);
    rtc_time.Minutes = (uint8_t)((expected % 3600U) / 60U);
    rtc_time.Seconds = (uint8_t)(expected % 60U);

    if (HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN) == HAL_OK)
    {
        RebaseReferences(expected);
        recovery_count++;
        status = RTC_MONITOR_HEALTHY;
        phase = RTC_PHASE_MONITORING;
    }
    else
    {
        failure_count++;
        status = RTC_MONITOR_LSE_FAILED;
        phase_started_cycles = total_cycles;
        phase = RTC_PHASE_RETRY_DELAY;
    }
}

static void ProcessMonitoring(void)
{
    uint64_t elapsed_cycles;
    uint32_t elapsed_seconds;
    uint32_t rtc_now;
    uint32_t rtc_elapsed;
    uint32_t minimum_expected;

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET)
    {
        BeginRecovery();
        return;
    }

    elapsed_cycles = total_cycles - sample_cycles;
    if (elapsed_cycles < CyclesForSeconds(RTC_MONITOR_WINDOW_SECONDS))
    {
        return;
    }

    elapsed_seconds = (uint32_t)(elapsed_cycles / core_clock_hz);
    rtc_now = ReadRtcCounter();
    rtc_elapsed = RtcElapsed(sample_rtc_seconds, rtc_now);
    minimum_expected = (elapsed_seconds > RTC_MONITOR_ALLOWED_LAG_SECONDS)
                     ? (elapsed_seconds - RTC_MONITOR_ALLOWED_LAG_SECONDS)
                     : 0U;

    sample_rtc_seconds = rtc_now;
    sample_cycles = total_cycles;

    if (rtc_elapsed < minimum_expected)
    {
        bad_windows++;
        status = RTC_MONITOR_SUSPECT;

        if (bad_windows >= RTC_MONITOR_BAD_WINDOWS)
        {
            BeginRecovery();
        }
        return;
    }

    /* Keep the long-term reference aligned while the RTC is running normally. */
    RebaseReferences(rtc_now);
    status = RTC_MONITOR_HEALTHY;
}

void RTC_Monitor_Init(void)
{
    uint32_t rtc_now;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    core_clock_hz = SystemCoreClock;
    last_cycle_counter = DWT->CYCCNT;
    total_cycles = 0U;
    recovery_count = 0U;
    failure_count = 0U;
    start_attempts = 0U;

    rtc_now = ReadRtcCounter();
    RebaseReferences(rtc_now);
    initialized = 1U;
    phase = RTC_PHASE_MONITORING;
    status = RTC_MONITOR_HEALTHY;
}

void RTC_Monitor_Process(void)
{
    if (initialized == 0U)
    {
        return;
    }

    UpdateCycleReference();

    if ((SystemCoreClock == 0U) || (SystemCoreClock != core_clock_hz))
    {
        core_clock_hz = SystemCoreClock;
        if (core_clock_hz == 0U)
        {
            return;
        }
        RebaseReferences(ReadRtcCounter());
    }

    switch (phase)
    {
        case RTC_PHASE_MONITORING:
            ProcessMonitoring();
            break;

        case RTC_PHASE_WAIT_LSE_OFF:
            if ((__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) ||
                ((total_cycles - phase_started_cycles) >= CyclesForMilliseconds(RTC_LSE_OFF_TIMEOUT_MS)))
            {
                StartLse();
            }
            break;

        case RTC_PHASE_WAIT_LSE_READY:
            if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) != RESET)
            {
                CLEAR_BIT(RTC->CRL, RTC_FLAG_RSF);
                phase_started_cycles = total_cycles;
                phase = RTC_PHASE_WAIT_LSE_SYNC;
            }
            else if ((total_cycles - phase_started_cycles) >= CyclesForSeconds(RTC_LSE_START_TIMEOUT_SECONDS))
            {
                RetryLseStartOrDelay();
            }
            break;

        case RTC_PHASE_WAIT_LSE_SYNC:
            if ((RTC->CRL & RTC_FLAG_RSF) != RESET)
            {
                RestoreExpectedTime();
            }
            else if ((__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) ||
                     ((total_cycles - phase_started_cycles) >= CyclesForSeconds(RTC_LSE_SYNC_TIMEOUT_SECONDS)))
            {
                RetryLseStartOrDelay();
            }
            break;

        case RTC_PHASE_RETRY_DELAY:
            if ((total_cycles - phase_started_cycles) >= CyclesForSeconds(RTC_LSE_RETRY_SECONDS))
            {
                BeginRecovery();
            }
            break;

        default:
            RTC_Monitor_Init();
            break;
    }
}

void RTC_Monitor_NotifyTimeChanged(void)
{
    if ((initialized == 0U) || (phase != RTC_PHASE_MONITORING))
    {
        return;
    }

    UpdateCycleReference();
    RebaseReferences(ReadRtcCounter());
    status = RTC_MONITOR_HEALTHY;
}

RTC_MonitorStatus RTC_Monitor_GetStatus(void)
{
    return status;
}

uint32_t RTC_Monitor_GetRecoveryCount(void)
{
    return recovery_count;
}

uint32_t RTC_Monitor_GetFailureCount(void)
{
    return failure_count;
}
