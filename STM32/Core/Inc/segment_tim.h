#ifndef SEGMENT_H
#define SEGMENT_H

#include <stdint.h>
#include "tim.h"

// Структура для хранения времени
typedef struct {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
} TimeStruct;

// Структура для управления сегментом
typedef struct {
  TIM_HandleTypeDef *htim;    // Указатель на таймер
  uint32_t channel;           // Канал (TIM_CHANNEL_1, TIM_CHANNEL_2, etc.)
  uint8_t isComplementary;    // 1 - комплементарный канал (CHxN), 0 - обычный
  

  
  // инициализируется само
  uint8_t isActive;           // активен ли сегмент, 0 - выключен, 1 - шимиться
  uint8_t Duty;               // скваэнлсть с которой надо шимить сегмент
  __IO uint32_t *ccmr_ptr;    // указатель на соответствующий CCMR регистр
  uint8_t ccmr_shift;    // сдвиг в регистре CCMR
} SegCtrl_t;

// ==================== НАСТРОЙКА ТАЙМЕРОВ ДЛЯ СЕГМЕНТОВ ====================
// Сегмент A
#define SEG_A_CONFIG {&htim1, TIM_CHANNEL_3, 1}

// Сегмент B
#define SEG_B_CONFIG {&htim3, TIM_CHANNEL_1, 0}

// Сегмент C
#define SEG_C_CONFIG {&htim2, TIM_CHANNEL_2, 0}

// Сегмент D
#define SEG_D_CONFIG {&htim4, TIM_CHANNEL_1, 0}

// Сегмент E
#define SEG_E_CONFIG {&htim4, TIM_CHANNEL_2, 0}

// Сегмент F
#define SEG_F_CONFIG {&htim4, TIM_CHANNEL_3, 0}

// Сегмент G
#define SEG_G_CONFIG {&htim4, TIM_CHANNEL_4, 0}

// ==================== ПУБЛИЧНЫЕ ФУНКЦИИ ====================

// Инициализация модуля
void Segment_Init(void);

// Установка времени для отображения
void Segment_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

// Установка глобальной яркости (0-100%)
void Segment_SetBrightness(uint8_t percent);

// Основная функция обновления дисплея (вызывается из прерывания таймера)
// Внутри сама рассчитывает когда переключать разряд
void Segment_Process(void);

#endif