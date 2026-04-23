#ifndef CLOCK_MANAGER_H
#define CLOCK_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"  // где определен time_t

// Инициализация
void ClockManager_Init(void);

// Получить текущее время из RTC
time_t ClockManager_GetTime(int blink);

// Установить время в RTC
void ClockManager_SetTime(uint8_t hour, uint8_t min, uint8_t sec);

// Получить яркость (0-10)
uint8_t ClockManager_GetDuty(void);

// Установить яркость (0-10) и сохранить
void ClockManager_SetDuty(uint8_t value);

// Сброс времени на 00:00:00
void ClockManager_ResetTime(void);

// LED control
void ClockManager_SetLEDState(uint8_t state);
uint8_t ClockManager_GetLEDState(void);

// PowerOn Song (DR5)
void ClockManager_SetPowerOnSong(uint8_t song);
uint8_t ClockManager_GetPowerOnSong(void);

// Alarm Song (DR6)
void ClockManager_SetAlarmSong(uint8_t song);
uint8_t ClockManager_GetAlarmSong(void);

// Menu Sound
void ClockManager_SetMenuSound(uint8_t enabled);
uint8_t ClockManager_GetMenuSound(void);
#endif
