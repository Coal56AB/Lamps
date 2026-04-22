#ifndef SEGMENT_H
#define SEGMENT_H

#include <stdint.h>

#define SEG_DIGITS_COUNT 6  // Количество разрядов на дисплее

// ==================== Инициализация ====================
// Инициализация модуля (обнуляет буферы и переменные)
void Segment_Init(void);

// ==================== Отображение ====================
// Установить символ в конкретный разряд (0..SEG_DIGITS_COUNT-1)
void Segment_SetChar(uint8_t pos, char c);

// Установить сразу строку (например "HELLO ")
void Segment_SetString(const char *str);

// Установить напрямую маску сегментов для разряда
void Segment_SetRaw(uint8_t pos, uint8_t mask);

// Установка времени для отображения
void Segment_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

// ==================== Яркость ====================
// Установка глобальной яркости (0-100%)
void Segment_SetBrightness(uint8_t percent);

// ==================== Обновление дисплея ====================
// Основная функция обновления дисплея
// Вызывается каждые PROCESS_INTERVAL_US микросекунд или из таймера
void Segment_Process(void);

#endif
