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
    uint32_t configuredSeconds;
    uint32_t remainingTime;
    uint32_t runStartedAt;
    uint32_t lastDisplayUpdate;
    uint8_t editStep;
    uint8_t state;           // 0 = настройка, 1 = запущен, 2 = пауза, 3 = завершен
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

static uint32_t Timer_GetRemainingMs(void) {
    uint32_t elapsed;

    if (g_timer.state != 1) {
        return g_timer.remainingTime;
    }

    elapsed = HAL_GetTick() - g_timer.runStartedAt;
    if (elapsed >= g_timer.remainingTime) {
        return 0;
    }
    return g_timer.remainingTime - elapsed;
}

static void Timer_FormatSeconds(uint32_t totalSeconds, char *buf) {
    uint32_t hours = totalSeconds / 3600U;
    uint32_t minutes = (totalSeconds % 3600U) / 60U;
    uint32_t seconds = totalSeconds % 60U;
    sprintf(buf, "%02u%02u%02u",
            (unsigned int)hours,
            (unsigned int)minutes,
            (unsigned int)seconds);
}

static uint8_t Timer_GetDigit(uint8_t pos) {
    uint32_t hours = g_timer.configuredSeconds / 3600U;
    uint32_t minutes = (g_timer.configuredSeconds % 3600U) / 60U;
    uint32_t seconds = g_timer.configuredSeconds % 60U;

    switch (pos) {
        case 0: return (uint8_t)(hours / 10U);
        case 1: return (uint8_t)(hours % 10U);
        case 2: return (uint8_t)(minutes / 10U);
        case 3: return (uint8_t)(minutes % 10U);
        case 4: return (uint8_t)(seconds / 10U);
        default: return (uint8_t)(seconds % 10U);
    }
}

static void Timer_SetDigit(uint8_t pos, uint8_t value) {
    uint32_t hours = g_timer.configuredSeconds / 3600U;
    uint32_t minutes = (g_timer.configuredSeconds % 3600U) / 60U;
    uint32_t seconds = g_timer.configuredSeconds % 60U;

    switch (pos) {
        case 0: hours = value * 10U + hours % 10U; break;
        case 1: hours = (hours / 10U) * 10U + value; break;
        case 2: minutes = value * 10U + minutes % 10U; break;
        case 3: minutes = (minutes / 10U) * 10U + value; break;
        case 4: seconds = value * 10U + seconds % 10U; break;
        case 5: seconds = (seconds / 10U) * 10U + value; break;
        default: return;
    }

    g_timer.configuredSeconds = hours * 3600U + minutes * 60U + seconds;
}

static void Timer_Reset(void) {
    g_timer.state = 0;
    g_timer.remainingTime = g_timer.configuredSeconds * 1000U;
    g_timer.editStep = 0;
    g_timer.blinkState = false;
    g_timer.lastBlink = HAL_GetTick();
    Melody_Stop(&melody);
    Menu_Refresh();
}

static void Timer_Start(void) {
    if (g_timer.configuredSeconds == 0) {
        return;
    }

    g_timer.remainingTime = g_timer.configuredSeconds * 1000U;
    g_timer.runStartedAt = HAL_GetTick();
    g_timer.state = 1;
    g_timer.blinkState = false;
    Menu_Refresh();
}

static void Timer_Finish(void) {
    g_timer.state = 3;
    g_timer.remainingTime = 0;

    if (!Melody_PlaySong(&melody, ClockManager_GetAlarmSong())) {
        Melody_Play(&melody, &SFX_Alarm, 240);
    }
    Menu_Refresh();
}

static void Display_Timer(void) {
    char buf[7];

    if (g_timer.state == 3) {
        Segment_SetString("DONE  ");
        return;
    }

    if (g_timer.state == 0) {
        Timer_FormatSeconds(g_timer.configuredSeconds, buf);
        if (g_timer.blinkState) {
            buf[g_timer.editStep] = ' ';
        }
    } else {
        uint32_t remaining = Timer_GetRemainingMs();
        Timer_FormatSeconds((remaining + 999U) / 1000U, buf);
    }
    Segment_SetString(buf);
}

void Timer_Update(void) {
    uint32_t tick = HAL_GetTick();

    if (g_timer.state == 1 && Timer_GetRemainingMs() == 0) {
        Timer_Finish();
    }

    if (g_timer.state == 0 && tick - g_timer.lastBlink >= 500U) {
        g_timer.lastBlink = tick;
        g_timer.blinkState = !g_timer.blinkState;
        if (Menu_GetCurrentNode() == &g_timerNode) {
            Menu_Refresh();
        }
    }

    if ((g_timer.state == 1 || g_timer.state == 2) &&
        tick - g_timer.lastDisplayUpdate >= 100U) {
        g_timer.lastDisplayUpdate = tick;
        if (Menu_GetCurrentNode() == &g_timerNode) {
            Menu_Refresh();
        }
    }
}

static void OnEnter_Timer(void) {
    g_currentActivity = &g_timerNode;
    g_rootMenu.parent = &g_timerNode;
    g_timer.lastBlink = HAL_GetTick();
    Menu_Refresh();
}

static void Timer_OnButton(Button_Type btn, bool longPress) {
    uint32_t tick = HAL_GetTick();

    if (longPress && btn == BUTTON_SELECT) {
        Menu_OpenMenu(&g_rootMenu);
        return;
    }

    if (g_timer.state == 0) {
        if (btn == BUTTON_UP || btn == BUTTON_DOWN) {
            uint8_t digit = Timer_GetDigit(g_timer.editStep);
            uint8_t limit = (g_timer.editStep == 2 || g_timer.editStep == 4) ? 6 : 10;

            if (btn == BUTTON_UP) {
                digit = (uint8_t)((digit + 1U) % limit);
            } else {
                digit = (digit == 0) ? (uint8_t)(limit - 1U) : (uint8_t)(digit - 1U);
            }
            Timer_SetDigit(g_timer.editStep, digit);
            g_timer.blinkState = false;
            g_timer.lastBlink = tick;
            Menu_Refresh();
        } else if (btn == BUTTON_SELECT) {
            if (g_timer.editStep < 5) {
                g_timer.editStep++;
                g_timer.blinkState = false;
                g_timer.lastBlink = tick;
                Menu_Refresh();
            } else {
                Timer_Start();
            }
        }
        return;
    }

    if (g_timer.state == 1 && btn == BUTTON_SELECT) {
        g_timer.remainingTime = Timer_GetRemainingMs();
        g_timer.state = 2;
        Menu_Refresh();
    } else if (g_timer.state == 2 && btn == BUTTON_SELECT) {
        g_timer.runStartedAt = tick;
        g_timer.state = 1;
        Menu_Refresh();
    } else if (g_timer.state == 3 && btn == BUTTON_SELECT) {
        Timer_Reset();
    } else if (btn == BUTTON_BACK) {
        Timer_Reset();
    }
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
    .onUpdate = NULL,
    .onButton = Timer_OnButton,
    .data = &g_timer
};
