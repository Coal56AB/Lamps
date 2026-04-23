#include "menu_items.h"
#include "clock.h"
#include "segment.h"
#include "clock_manager.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint32_t startTime;
    uint32_t elapsedTime;    // Накопленное время при паузе
    bool running;
    uint8_t state;           // 0 = Сброшен, 1 = Запущен, 2 = Пауза
} StopwatchData;

typedef struct {
    uint32_t targetTime;     // Целевое время в мс
    uint32_t startTime;
    uint32_t elapsedTime;    // Накопленное время при паузе
    uint8_t editStep;        // 0-5: разряды, 6: готово
    uint8_t state;           // 0 = Настройка, 1 = Запущен, 2 = Пауза, 3 = Завершен
    bool running;
    bool blinkState;
    uint32_t lastBlink;
} TimerData;

static StopwatchData g_stopwatch;
static TimerData g_timer;

// Вспомогательная функция для форматирования мм:сс:цс
static void FormatTime(uint32_t ms, char* buf) {
    uint32_t total_sec = ms / 1000;
    uint32_t minutes = total_sec / 60;
    uint32_t seconds = total_sec % 60;
    uint32_t centiseconds = (ms % 1000) / 10;
    
    if (minutes > 0) {
        sprintf(buf, "%2d%02d%02d", minutes, seconds, centiseconds);
    } else { 
        sprintf(buf, "  %2d%02d", seconds, centiseconds);
    }
}

/////// CLOCK ////////
static void Display_Clock(void) {
    time_t now = ClockManager_GetTime(1);
    char buf[7];
    sprintf(buf, "%02d%02d%02d", now.hour, now.min, now.sec);
    Segment_SetString(buf);
}

static void OnEnter_Clock(void) {
    g_currentActivity = &g_clockNode;
    g_rootMenu.parent = &g_clockNode;
    Menu_Refresh();
}

static void Clock_OnButton(Button_Type btn, bool longPress) {
    if (longPress && btn == BUTTON_SELECT) {
        Menu_OpenMenu(&g_rootMenu);
        return;
    }
    
    if (btn == BUTTON_BACK) {
        Menu_GoBack();
    }
}

/////// СЕКУНДОМЕР ////////
static void Display_Stopwatch(void) {
    char buf[7];
    uint32_t current_ms = 0;
    
    switch (g_stopwatch.state) {
        case 0: // Сброшен
            sprintf(buf, "   000");
            break;
        case 1: // Запущен
            current_ms = (HAL_GetTick() - g_stopwatch.startTime) + g_stopwatch.elapsedTime;
            FormatTime(current_ms, buf);
            break;
        case 2: // Пауза
            FormatTime(g_stopwatch.elapsedTime, buf);
            break;
        default:
            sprintf(buf, "ERROR");
            break;
    }
    Segment_SetString(buf);
}

static void OnUpdate_Stopwatch(void) {
    // Для секундомера не нужен отдельный update,
    // т.к. Display обновляется по таймеру меню
    static uint32_t lastUpdate = 0;
    if (g_stopwatch.state == 1) {
        uint32_t tick = HAL_GetTick();
        if (tick - lastUpdate >= 10) {  // Обновление каждые 10 мс
            lastUpdate = tick;
            Menu_Refresh();
        }
    }
}

static void OnEnter_Stopwatch(void) {
    g_currentActivity = &g_stopwatchNode;
    g_rootMenu.parent = &g_stopwatchNode;
    g_stopwatch.state = 0;
    g_stopwatch.running = false;
    g_stopwatch.elapsedTime = 0;
    Menu_Refresh();
}

static void Stopwatch_OnButton(Button_Type btn, bool longPress) {
    if (longPress && btn == BUTTON_SELECT) {
        Menu_OpenMenu(&g_rootMenu);
        return;
    }
    
    uint32_t tick = HAL_GetTick();
    
    if (g_stopwatch.state == 0 && btn == BUTTON_SELECT) {
        // Старт
        g_stopwatch.state = 1;
        g_stopwatch.running = true;
        g_stopwatch.startTime = tick;
        g_stopwatch.elapsedTime = 0;
        Menu_Refresh();
    }
    else if (g_stopwatch.state == 1 && btn == BUTTON_SELECT) {
        // Пауза
        g_stopwatch.state = 2;
        g_stopwatch.running = false;
        g_stopwatch.elapsedTime += tick - g_stopwatch.startTime;
        Menu_Refresh();
    }
    else if (g_stopwatch.state == 2 && btn == BUTTON_SELECT) {
        // Продолжить
        g_stopwatch.state = 1;
        g_stopwatch.running = true;
        g_stopwatch.startTime = tick;
        Menu_Refresh();
    }
    else if ((g_stopwatch.state == 1 || g_stopwatch.state == 2) && btn == BUTTON_BACK) {
        // Сброс
        g_stopwatch.state = 0;
        g_stopwatch.running = false;
        g_stopwatch.elapsedTime = 0;
        Menu_Refresh();
    }
}

/////// ТАЙМЕР ////////

static void Display_Timer(void) {
  Segment_SetString("FUTURE");
}

static void OnUpdate_Timer(void) {
}

static void OnEnter_Timer(void) {
}

static void Timer_OnButton(Button_Type btn, bool longPress) {
}

// NODES
MenuNode g_clockNode = {
    .name = "CLOCH",
    .parent = NULL,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .needsRedraw = 1,
    .display = Display_Clock,
    .onEnter = OnEnter_Clock,
    .onUpdate = NULL,
    .onButton = Clock_OnButton,
    .data = NULL
};

MenuNode g_stopwatchNode = {
    .name = "SECOND",
    .parent = NULL,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .needsRedraw = 1,
    .display = Display_Stopwatch,
    .onEnter = OnEnter_Stopwatch,
    .onUpdate = OnUpdate_Stopwatch,
    .onButton = Stopwatch_OnButton,
    .data = &g_stopwatch
};

MenuNode g_timerNode = {
    .name = "TIHER",
    .parent = NULL,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .needsRedraw = 1,
    .display = Display_Timer,
    .onEnter = OnEnter_Timer,
    .onUpdate = OnUpdate_Timer,
    .onButton = Timer_OnButton,
    .data = &g_timer
};
