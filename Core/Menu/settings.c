#include "menu_items.h"
#include "settings.h"
#include "segment.h"
#include "clock_manager.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    time_t editTime;
    uint8_t editStep;
    bool blinkState;
    uint32_t lastBlink;
} TimeEditData;

static TimeEditData g_timeData;
static uint8_t g_originalDuty;
static uint8_t g_editDuty;

/////// TIME EDIT ////////
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

static void OnEnter_TimeEdit(void) {
    g_timeData.editTime = ClockManager_GetTime(0);
    g_timeData.editStep = 0;
    g_timeData.blinkState = true;
    g_timeData.lastBlink = HAL_GetTick();
}
static void OnUpdate_TimeEdit(void) {
    uint32_t tick = HAL_GetTick();
    if (tick - g_timeData.lastBlink >= 500) {
        g_timeData.lastBlink = tick;
        g_timeData.blinkState = !g_timeData.blinkState;
        Menu_Refresh();
    }
}
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



/////// DUTY EDIT ////////
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
static void OnEnter_DutyEdit(void) {
    g_originalDuty = ClockManager_GetDuty();
    g_editDuty = g_originalDuty;
    Segment_SetBrightness(g_editDuty * 10);
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




/////// RESET ////////
static void Display_Reset(void) {
    Segment_SetString("RESET ");
}
static void OnEnter_Reset(void) {}
static void Reset_OnButton(Button_Type btn, bool longPress) {
  
    if ((btn == BUTTON_SELECT) && longPress) {
        ClockManager_ResetTime();
        ClockManager_SetDuty(5);
        Menu_GoBack();
    }
}

/////// LED EDIT ////////
static void Display_LEDEdit(void) {
    if (ClockManager_GetLEDState()) {
        Segment_SetString("LED  1");
    } else {
        Segment_SetString("LED  0");
    }
}

static void LEDEdit_OnButton(Button_Type btn, bool longPress) {
    (void)longPress;
    uint8_t newState;
    switch (btn) {
        case BUTTON_SELECT:
            newState = !ClockManager_GetLEDState();
            ClockManager_SetLEDState(newState);
            Menu_Refresh();
            break;
        case BUTTON_BACK:
            Menu_GoBack();
            break;
        default:
            break;
    }
}


/////// POWER ON SONG ////////
static void Display_PowerOnSong(void) {
    char buf[7];
    sprintf(buf, "PnS%02d", ClockManager_GetPowerOnSong());
    Segment_SetString(buf);
}
static void PowerOnSong_OnButton(Button_Type btn, bool longPress) {
    (void)longPress;
    uint8_t current = ClockManager_GetPowerOnSong();
    
    switch (btn) {
        case BUTTON_UP:
            if (current < 10) {
                current++;
                ClockManager_SetPowerOnSong(current);
                Menu_Refresh();
            }
            break;
        case BUTTON_DOWN:
            if (current > 0) {
                current--;
                ClockManager_SetPowerOnSong(current);
                Menu_Refresh();
            }
            break;
        case BUTTON_SELECT:
        case BUTTON_BACK:
            Menu_GoBack();
            break;
        default:
            break;
    }
}

/////// ALARM SONG ////////
static void Display_AlarmSong(void) {
    char buf[7];
    sprintf(buf, "AL%02d", ClockManager_GetAlarmSong());
    Segment_SetString(buf);
}

static void AlarmSong_OnButton(Button_Type btn, bool longPress) {
    (void)longPress;
    uint8_t current = ClockManager_GetAlarmSong();
    
    switch (btn) {
        case BUTTON_UP:
            if (current < 10) {
                current++;
                ClockManager_SetAlarmSong(current);
                Menu_Refresh();
            }
            break;
        case BUTTON_DOWN:
            if (current > 0) {
                current--;
                ClockManager_SetAlarmSong(current);
                Menu_Refresh();
            }
            break;
        case BUTTON_SELECT:
        case BUTTON_BACK:
            Menu_GoBack();
            break;
        default:
            break;
    }
}

/////// ALARM SONG ////////
static void Display_MenuSound(void) {
    if (ClockManager_GetMenuSound()) {
        Segment_SetString("BEEP 1");
    } else {
        Segment_SetString("BEEP 0");
    }
}

static void MenuSound_OnButton(Button_Type btn, bool longPress) {
    (void)longPress;
    uint8_t newState;
    switch (btn) {
        case BUTTON_SELECT:
            newState = !ClockManager_GetMenuSound();
            ClockManager_SetMenuSound(newState);
            Menu_Refresh();
            break;
        case BUTTON_BACK:
            Menu_GoBack();
            break;
        default:
            break;
    }
}

// NODES// NODES
MenuNode g_timeEditNode = {
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

MenuNode g_dutyEditNode = {
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

MenuNode g_LEDEditNode = {
    .name = "LED",
    .parent = &g_settingsNode,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .display = Display_LEDEdit,
    .onEnter = NULL,
    .onUpdate = NULL,
    .onButton = LEDEdit_OnButton,
    .data = NULL
};

MenuNode g_MenuSoundNode = {
    .name = "BEEP",
    .parent = &g_settingsNode,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .display = Display_MenuSound,
    .onEnter = NULL,
    .onUpdate = NULL,
    .onButton = MenuSound_OnButton,
    .data = NULL
};

MenuNode g_PowerOnSongNode = {
    .name = "PonS",
    .parent = &g_settingsNode,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .display = Display_PowerOnSong,
    .onEnter = NULL,
    .onUpdate = NULL,
    .onButton = PowerOnSong_OnButton,
    .data = NULL
};

MenuNode g_AlarmSongNode = {
    .name = "AL",
    .parent = &g_settingsNode,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .display = Display_AlarmSong,
    .onEnter = NULL,
    .onUpdate = NULL,
    .onButton = AlarmSong_OnButton,
    .data = NULL
};

MenuNode g_resetNode = {
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

MenuNode* g_settingsChildren[] = {
    &g_timeEditNode,
    &g_dutyEditNode,
    &g_LEDEditNode,
    &g_MenuSoundNode,
    &g_PowerOnSongNode,
    &g_AlarmSongNode,
    &g_resetNode
};
