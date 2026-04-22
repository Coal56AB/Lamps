#include "menu_items.h"
#include "segment.h"
#include "clock_manager.h"
#include "games.h"
#include "clock.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
MenuNode g_rootMenu;
// объявляем узлы меню
MenuNode g_gamesNode;
MenuNode g_settingsNode;
MenuNode g_timeEditNode;
MenuNode g_dutyEditNode;
MenuNode g_LEDEditNode;
MenuNode g_MenuSoundNode;
MenuNode g_PowerOnSongNode;
MenuNode g_SongNode;
MenuNode g_resetNode;


// ==================== Данные ====================
typedef struct {
    time_t editTime;
    uint8_t editStep;
    bool blinkState;
    uint32_t lastBlink;
} TimeEditData;

static TimeEditData g_timeData;
static uint8_t g_originalDuty;
static uint8_t g_editDuty;

// Текущая активность в корне
MenuNode* g_currentActivity = NULL;

// ==================== Функции отображения активностей ====================

static void Display_MenuItem(void) {
    MenuNode* current = Menu_GetCurrentNode();
    if (current && current->selectedChild < current->childCount) {
        const char* name = current->children[current->selectedChild]->name;
        char buf[7];
        sprintf(buf, "%-6.6s", name);
        Segment_SetString(buf);
    }
}

static void Display_TimeEdit(void) {
    char buf[7];
    sprintf(buf, "%02d%02d%02d", 
            g_timeData.editTime.hour, 
            g_timeData.editTime.min, 
            g_timeData.editTime.sec);
    if (g_timeData.blinkState && g_timeData.editStep < 6) {
        buf[g_timeData.editStep] = ' ';
    }
    Segment_SetString(buf);
}

static void Display_DutyEdit(void) {
    char buf[7] = "DUTY  ";
    if (g_editDuty == 10) {
        buf[4] = '1';
        buf[5] = '0';
    } else {
        buf[5] = '0' + g_editDuty;
    }
    Segment_SetString(buf);
}

static void Display_Reset(void) {
    Segment_SetString("RESET ");
}

// ==================== Вход в активности ====================

static void OnEnter_TimeEdit(void) {
    g_timeData.editTime = ClockManager_GetTime(0);
    g_timeData.editStep = 0;
    g_timeData.blinkState = true;
    g_timeData.lastBlink = HAL_GetTick();
}

static void OnEnter_DutyEdit(void) {
    g_originalDuty = ClockManager_GetDuty();
    g_editDuty = g_originalDuty;
    Segment_SetBrightness(g_editDuty * 10);
}

static void OnEnter_Reset(void) {}

// ==================== Обновления ====================
static void OnUpdate_TimeEdit(void) {
    uint32_t tick = HAL_GetTick();
    if (tick - g_timeData.lastBlink >= 500) {
        g_timeData.lastBlink = tick;
        g_timeData.blinkState = !g_timeData.blinkState;
        Menu_Refresh();
    }
}

// ==================== Обработчики кнопок активностей ====================
static void TimeEdit_OnButton(Button_Type btn, bool longPress) {
    (void)longPress;
  
    switch (btn) {
        case BUTTON_UP: {
            uint8_t tens, units;
            switch (g_timeData.editStep) {
                case 0:
                    tens = g_timeData.editTime.hour / 10;
                    units = g_timeData.editTime.hour % 10;
                    tens = (tens + 1) % 3;
                    g_timeData.editTime.hour = tens * 10 + units;
                    break;
                case 1:
                    tens = g_timeData.editTime.hour / 10;
                    units = (g_timeData.editTime.hour % 10 + 1) % 10;
                    if (tens == 2 && units > 3) units = 0;
                    g_timeData.editTime.hour = tens * 10 + units;
                    break;
                case 2:
                    tens = (g_timeData.editTime.min / 10 + 1) % 6;
                    g_timeData.editTime.min = tens * 10 + (g_timeData.editTime.min % 10);
                    break;
                case 3:
                    units = (g_timeData.editTime.min % 10 + 1) % 10;
                    g_timeData.editTime.min = (g_timeData.editTime.min / 10) * 10 + units;
                    break;
                case 4:
                    tens = (g_timeData.editTime.sec / 10 + 1) % 6;
                    g_timeData.editTime.sec = tens * 10 + (g_timeData.editTime.sec % 10);
                    break;
                case 5:
                    units = (g_timeData.editTime.sec % 10 + 1) % 10;
                    g_timeData.editTime.sec = (g_timeData.editTime.sec / 10) * 10 + units;
                    break;
            }
            Menu_Refresh();
            break;
        }
        case BUTTON_DOWN: {
            uint8_t tens, units;
            switch (g_timeData.editStep) {
                case 0:
                    tens = g_timeData.editTime.hour / 10;
                    units = g_timeData.editTime.hour % 10;
                    tens = (tens == 0) ? 2 : tens - 1;
                    g_timeData.editTime.hour = tens * 10 + units;
                    break;
                case 1:
                    tens = g_timeData.editTime.hour / 10;
                    units = g_timeData.editTime.hour % 10;
                    if (units == 0) units = (tens == 2) ? 3 : 9;
                    else units--;
                    g_timeData.editTime.hour = tens * 10 + units;
                    break;
                case 2:
                    tens = g_timeData.editTime.min / 10;
                    tens = (tens == 0) ? 5 : tens - 1;
                    g_timeData.editTime.min = tens * 10 + (g_timeData.editTime.min % 10);
                    break;
                case 3:
                    units = g_timeData.editTime.min % 10;
                    units = (units == 0) ? 9 : units - 1;
                    g_timeData.editTime.min = (g_timeData.editTime.min / 10) * 10 + units;
                    break;
                case 4:
                    tens = g_timeData.editTime.sec / 10;
                    tens = (tens == 0) ? 5 : tens - 1;
                    g_timeData.editTime.sec = tens * 10 + (g_timeData.editTime.sec % 10);
                    break;
                case 5:
                    units = g_timeData.editTime.sec % 10;
                    units = (units == 0) ? 9 : units - 1;
                    g_timeData.editTime.sec = (g_timeData.editTime.sec / 10) * 10 + units;
                    break;
            }
            Menu_Refresh();
            break;
        }
        case BUTTON_SELECT:
            g_timeData.editStep++;
            if (g_timeData.editStep >= 6) {
                ClockManager_SetTime(g_timeData.editTime.hour, 
                                    g_timeData.editTime.min, 
                                    g_timeData.editTime.sec);
                Menu_GoBack();
            }
            Menu_Refresh();
            break;
        case BUTTON_BACK:
          Menu_GoBack();
        default:
            break;
    }
}

static void DutyEdit_OnButton(Button_Type btn, bool longPress) {
    (void)longPress;
    switch (btn) {
        case BUTTON_UP:
            if (g_editDuty < 10) {
                g_editDuty++;
                Segment_SetBrightness(g_editDuty * 10);
                Menu_Refresh();
            }
            break;
        case BUTTON_DOWN:
            if (g_editDuty > 0) {
                g_editDuty--;
                Segment_SetBrightness(g_editDuty * 10);
                Menu_Refresh();
            }
            break;
        case BUTTON_SELECT:
            ClockManager_SetDuty(g_editDuty);
            Menu_GoBack();
            break;
        case BUTTON_BACK:
            ClockManager_SetDuty(g_originalDuty);
            Menu_GoBack();
        default:
            break;
    }
}

static void Reset_OnButton(Button_Type btn, bool longPress) {
  
    if ((btn == BUTTON_SELECT) && longPress) {
        ClockManager_ResetTime();
        ClockManager_SetDuty(5);
        Menu_GoBack();
    }
}

// ==================== Узлы меню ====================
MenuNode g_gamesNode;
MenuNode g_settingsNode;
MenuNode g_timeEditNode;
MenuNode g_dutyEditNode;
MenuNode g_LEDEditNode;
MenuNode g_MenuSoundNode;
MenuNode g_PowerOnSongNode;
MenuNode g_SongNode;
MenuNode g_resetNode;

static MenuNode* g_settingsChildren[] = {
    &g_timeEditNode,
    &g_dutyEditNode,
    &g_LEDEditNode,
    &g_MenuSoundNode,
    &g_PowerOnSongNode,
    &g_SongNode,
    &g_resetNode
};

static MenuNode* g_mainMenuChildren[] = {
    &g_clockNode,
    &g_timerNode,
    &g_stopwatchNode,
    &g_gamesNode,
    &g_settingsNode
};

MenuNode g_rootMenu = {
    .name = "MAIN",
    .parent = NULL,
    .children = g_mainMenuChildren,
    .childCount = 5,
    .selectedChild = 0,
    .display = Display_MenuItem,
    .onEnter = NULL,
    .onUpdate = NULL,
    .onButton = NULL,
    .data = NULL
};

void MenuItems_Init(void) {
    g_gamesNode = (MenuNode){
        .name = "PLAY",
        .parent = &g_rootMenu,
        .children = g_gamesChildren,
        .childCount = sizeof(g_gamesChildren)/sizeof(g_gamesChildren[0]),
        .selectedChild = 0,
        .display = Display_MenuItem,
        .onEnter = NULL,
        .onUpdate = NULL,
        .onButton = NULL,
        .data = NULL
    };
    
    g_settingsNode = (MenuNode){
        .name = "SETUP",
        .parent = &g_rootMenu,
        .children = g_settingsChildren,
        .childCount = sizeof(g_settingsChildren)/sizeof(g_settingsChildren[0]),
        .selectedChild = 0,
        .display = Display_MenuItem,
        .onEnter = NULL,
        .onUpdate = NULL,
        .onButton = NULL,
        .data = NULL
    };
    
    g_timeEditNode = (MenuNode){
        .name = "SET T",
        .parent = &g_settingsNode,
        .children = NULL,
        .childCount = 0,
        .selectedChild = 0,
        .display = Display_TimeEdit,
        .onEnter = OnEnter_TimeEdit,
        .onUpdate = OnUpdate_TimeEdit,
        .onButton = TimeEdit_OnButton,
        .data = &g_timeData
    };
    
    g_dutyEditNode = (MenuNode){
        .name = "SET D",
        .parent = &g_settingsNode,
        .children = NULL,
        .childCount = 0,
        .selectedChild = 0,
        .display = Display_DutyEdit,
        .onEnter = OnEnter_DutyEdit,
        .onUpdate = NULL,
        .onButton = DutyEdit_OnButton,
        .data = &g_editDuty
    };
    
    g_resetNode = (MenuNode){
        .name = "RESET",
        .parent = &g_settingsNode,
        .children = NULL,
        .childCount = 0,
        .selectedChild = 0,
        .display = Display_Reset,
        .onEnter = OnEnter_Reset,
        .onUpdate = NULL,
        .onButton = Reset_OnButton,
        .data = NULL
    };
}

MenuNode* Menu_GetRootMenu(void) {
    return &g_rootMenu;
}

MenuNode* Menu_GetCurrentActivity(void) {
    return g_currentActivity;
}
