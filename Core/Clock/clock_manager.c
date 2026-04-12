#include "clock_manager.h"
#include "segment.h"
#include "rtc.h"

static uint8_t dutyValue = 5;
static time_t currentTime;
RTC_TimeTypeDef rtc_time;

// Яркость в RTC Backup Register (используем BKP_DR1)
static void SaveDuty(void) {
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, dutyValue);
}

static void LoadDuty(void) {
    uint32_t val = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
    dutyValue = (val <= 10 && val > 0) ? (uint8_t)val : 5;
}

void ClockManager_Init(void) {
    LoadDuty();
    Segment_SetBrightness(dutyValue * 10);
    
    // Если RTC не инициализирован - сброс времени
    if (HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN) != HAL_OK) {
        ClockManager_ResetTime();
    }
}

time_t ClockManager_GetTime(void) {
    HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
    currentTime.hour = rtc_time.Hours;
    currentTime.min = rtc_time.Minutes;
    currentTime.sec = rtc_time.Seconds;
    return currentTime;
}

void ClockManager_SetTime(uint8_t hour, uint8_t min, uint8_t sec) {
    RTC_TimeTypeDef rtc_time;
    rtc_time.Hours = hour;
    rtc_time.Minutes = min;
    rtc_time.Seconds = sec;
//    rtc_time.TimeFormat = RTC_HOURFORMAT_24;
//    rtc_time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
//    rtc_time.StoreOperation = RTC_STOREOPERATION_RESET;
    HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
}

uint8_t ClockManager_GetDuty(void) {
    return dutyValue;
}

void ClockManager_SetDuty(uint8_t value) {
    if (value > 10) value = 10;
    dutyValue = value;
    Segment_SetBrightness(dutyValue * 10);
    SaveDuty();
}

void ClockManager_ResetTime(void) {
    ClockManager_SetTime(0, 0, 0);
}
