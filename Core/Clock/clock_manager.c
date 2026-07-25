#include "clock_manager.h"
#include "segment.h"
#include "rtc.h"

static uint8_t dutyValue = 5;
static uint8_t fadeValue = 5;
static uint8_t volumeValue = 5;
static time_t currentTime;
static uint8_t led_enable;
RTC_TimeTypeDef rtc_time;

#define BKP_DR_DUTY         RTC_BKP_DR1
#define BKP_DR_LED_STATE    RTC_BKP_DR2
#define BKP_DR_POWERON_SONG RTC_BKP_DR5
#define BKP_DR_ALARM_SONG   RTC_BKP_DR6
#define BKP_DR_MENU_SOUND   RTC_BKP_DR3
#define BKP_DR_FADE         RTC_BKP_DR4
#define BKP_DR_INIT         RTC_BKP_DR7
#define BKP_DR_VOLUME       RTC_BKP_DR8
#define BKP_DR_VOLUME_INIT  RTC_BKP_DR9
#define BKP_INIT_MAGIC      0x4C50U
#define BKP_VOLUME_MAGIC    0x564FU

// Яркость в RTC Backup Register (используем BKP_DR1)
static void SaveDuty(void) {
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_DUTY, dutyValue);
}

static void LoadDuty(void) {
    uint32_t val = HAL_RTCEx_BKUPRead(&hrtc, BKP_DR_DUTY);
    dutyValue = (val <= 10 && val > 0) ? (uint8_t)val : 5;
}

static void SaveFade(void) {
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_FADE, fadeValue);
}

static void LoadFade(void) {
    uint32_t val = HAL_RTCEx_BKUPRead(&hrtc, BKP_DR_FADE);
    fadeValue = (val <= 10) ? (uint8_t)val : 5;
}

void ClockManager_Init(void) {
    if (HAL_RTCEx_BKUPRead(&hrtc, BKP_DR_INIT) != BKP_INIT_MAGIC) {
        /*
         * A reset backup domain contains zeroes. Some zeroes are valid user
         * settings, so individual range checks cannot distinguish the first
         * start from saved configuration. Initialize the whole domain and
         * write the marker last.
         */
        dutyValue = 5;
        fadeValue = 5;
        volumeValue = 5;
        led_enable = 1;

        ClockManager_ResetTime();
        SaveDuty();
        SaveFade();
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_LED_STATE, 1);
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_MENU_SOUND, 1);
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_POWERON_SONG, 0);
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_ALARM_SONG, 0);
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_VOLUME, volumeValue);
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_VOLUME_INIT, BKP_VOLUME_MAGIC);
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_INIT, BKP_INIT_MAGIC);
    } else {
        LoadDuty();
        LoadFade();
        led_enable = ClockManager_GetLEDState();

        if (HAL_RTCEx_BKUPRead(&hrtc, BKP_DR_VOLUME_INIT) != BKP_VOLUME_MAGIC) {
            volumeValue = 5;
            HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_VOLUME, volumeValue);
            HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_VOLUME_INIT, BKP_VOLUME_MAGIC);
        } else {
            volumeValue = ClockManager_GetVolume();
        }
    }

    Segment_SetBrightness(dutyValue * 10);
    Segment_SetFade(fadeValue);

    ClockManager_GetMenuSound();
}

time_t ClockManager_GetTime(int blink) {
    HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
    currentTime.hour = rtc_time.Hours;
    currentTime.min = rtc_time.Minutes;
    if(blink)
    {
      if(currentTime.sec != rtc_time.Seconds)
      {
        if(led_enable)
          HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        else
          HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
      }
    }
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

uint8_t ClockManager_GetFade(void) {
    return fadeValue;
}

void ClockManager_SetFade(uint8_t value) {
    if (value > 10) value = 10;
    fadeValue = value;
    Segment_SetFade(fadeValue);
    SaveFade();
}

void ClockManager_ResetTime(void) {
    ClockManager_SetTime(0, 0, 0);
}


// LED State in Backup Register

void ClockManager_SetLEDState(uint8_t state) {
    if (state) {
        led_enable = 1;
    } else {
        led_enable = 0;
    }
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_LED_STATE, state ? 1 : 0);
}

uint8_t ClockManager_GetLEDState(void) {
    uint32_t led_enable = HAL_RTCEx_BKUPRead(&hrtc, BKP_DR_LED_STATE);
    if (led_enable == 0 || led_enable == 1) {
        // Сохраненное значение корректно
    } else {
        led_enable = 1;  // По умолчанию LED включен
    }
    return (uint8_t)led_enable;
}

// PowerOn Song in Backup Register
void ClockManager_SetPowerOnSong(uint8_t song) {
    if (song > 10) song = 10;
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_POWERON_SONG, song);
}

uint8_t ClockManager_GetPowerOnSong(void) {
    uint32_t val = HAL_RTCEx_BKUPRead(&hrtc, BKP_DR_POWERON_SONG);
    if (val > 10) val = 0;
    return (uint8_t)val;
}

// Alarm Song in Backup Register

void ClockManager_SetAlarmSong(uint8_t song) {
    if (song > 10) song = 10;
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_ALARM_SONG, song);
}

uint8_t ClockManager_GetAlarmSong(void) {
    uint32_t val = HAL_RTCEx_BKUPRead(&hrtc, BKP_DR_ALARM_SONG);
    if (val > 10) val = 0;
    return (uint8_t)val;
}

// Menu Sound State in Backup Register
extern uint8_t g_sound_enabled;

void ClockManager_SetMenuSound(uint8_t enabled) {
    g_sound_enabled = enabled ? 1 : 0;
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_MENU_SOUND, g_sound_enabled);
}

uint8_t ClockManager_GetMenuSound(void) {
    uint32_t val = HAL_RTCEx_BKUPRead(&hrtc, BKP_DR_MENU_SOUND);
    g_sound_enabled = (val == 0 || val == 1) ? (uint8_t)val : 1;
    return g_sound_enabled;
}

void ClockManager_SetVolume(uint8_t volume) {
    if (volume > 10) volume = 10;
    volumeValue = volume;
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_DR_VOLUME, volumeValue);
}

uint8_t ClockManager_GetVolume(void) {
    uint32_t val = HAL_RTCEx_BKUPRead(&hrtc, BKP_DR_VOLUME);
    volumeValue = (val <= 10) ? (uint8_t)val : 5;
    return volumeValue;
}
