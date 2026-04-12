#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_SELECT,
    BUTTON_BACK,
    BUTTON_COUNT
} Button_Type;

typedef enum {
    STATE_CLOCK,           // режим часов
    STATE_MAIN_MENU,       // главное меню (выбор пункта)
    STATE_SET_TIME,        // настройка времени
    STATE_SET_DUTY,        // настройка яркости
    STATE_RESET_CONFIRM    // подтверждение сброса
} SystemState;

// Чтение кнопки (реализуется в main.c или hal)
int ReadButton(int btn);

void Menu_Init(void);
void Menu_Process(void);

#endif