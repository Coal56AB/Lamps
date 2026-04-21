#include "menu_items.h"
#include "segment.h"
#include "clock_manager.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
static MenuNode g_rootMenu;
// Сначала объявляем узлы активностей
MenuNode g_clockNode;
MenuNode g_timerNode;
MenuNode g_stopwatchNode;
MenuNode g_ZeroMillisNode;
MenuNode g_ReactionTimeNode;
MenuNode g_ClickerTimeNode;

// Потом объявляем узлы меню
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

typedef struct {
    uint32_t startTime;
    uint32_t pressTime;
    bool celebrating;
    bool result;
    int32_t diff_ms;
    uint8_t state;
} Game1SecData;

typedef struct {
    uint32_t waitStart;
    uint32_t ledOnTime;
    uint32_t reactionTime;
    uint8_t state;
} GameReactionData;

typedef struct {
    uint32_t endTime;
    uint16_t clicks;
    bool active;
    bool finished;
} GameClickerData;

static TimeEditData g_timeData;
static uint8_t g_originalDuty;
static uint8_t g_editDuty;
static Game1SecData g_game1sec;
static GameReactionData g_gameReaction;
static GameClickerData g_gameClicker;

// Текущая активность в корне
static MenuNode* g_currentActivity = NULL;

// ==================== Функции отображения активностей ====================
static void Display_Clock(void) {
    time_t now = ClockManager_GetTime(1);
    char buf[7];
    sprintf(buf, "%02d%02d%02d", now.hour, now.min, now.sec);
    Segment_SetString(buf);
}

static void Display_Timer(void) {
    Segment_SetString("TIMER ");
}

static void Display_Stopwatch(void) {
    Segment_SetString("SECOND ");
}

static void Display_ZeroMillis(void) {
    char buf[7];
    switch (g_game1sec.state) {
        case 0: 
          g_game1sec.celebrating = 0;
          sprintf(buf, "START"); break;
        case 1:
          g_game1sec.diff_ms = (HAL_GetTick() - g_game1sec.startTime);
        case 2: 
          sprintf(buf, "  %4d", g_game1sec.diff_ms/10); 
          break;
        default: sprintf(buf, "ERROR"); break;
    }
    if(g_game1sec.state == 2)
    {
      if((g_game1sec.diff_ms/10)%100 == 0)
      {
        if(!g_game1sec.celebrating)
        {
          g_game1sec.celebrating = 1;
          Melody_Play(&melody, &Polyphia_PlayingGod, 134);
        }
      }
    }
    Segment_SetString(buf);
}

static void Display_Reaction(void) {
    char buf[7];
    switch (g_gameReaction.state) {
        case 0: sprintf(buf, "START"); break;
        case 1: sprintf(buf, "      "); break;
        case 2: sprintf(buf, "888888"); break;
        case 3: sprintf(buf, "%6d", g_gameReaction.reactionTime); break;
        default: sprintf(buf, "ERROR"); break;
    }
    Segment_SetString(buf);
}

static void Display_Clicker(void) {
    char buf[7];
    if (!g_gameClicker.active && !g_gameClicker.finished) {
        sprintf(buf, "START");
    } else if (g_gameClicker.active) {
        uint32_t remaining = (g_gameClicker.endTime - HAL_GetTick()) / 1000;
        sprintf(buf, "%2d %3d", remaining, g_gameClicker.clicks);
    } else {
        sprintf(buf, "   %3d", g_gameClicker.clicks);
    }
    Segment_SetString(buf);
}

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
static void OnEnter_Clock(void) {
    g_currentActivity = &g_clockNode;
    g_rootMenu.parent = &g_clockNode;
}

static void OnEnter_Timer(void) {
    g_currentActivity = &g_timerNode;
    g_rootMenu.parent = &g_timerNode;
}

static void OnEnter_Stopwatch(void) {
    g_currentActivity = &g_stopwatchNode;
    g_rootMenu.parent = &g_stopwatchNode;
}

static void OnEnter_ZeroMillis(void) {
    g_game1sec.state = 0;
    Menu_Refresh();
}

static void OnEnter_Reaction(void) {
    g_gameReaction.state = 0;
    Menu_Refresh();
}

static void OnEnter_Clicker(void) {
    g_gameClicker.active = false;
    g_gameClicker.finished = false;
    g_gameClicker.clicks = 0;
    Menu_Refresh();
}

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

static void OnUpdate_Reaction(void) {
    uint32_t tick = HAL_GetTick();
    if (g_gameReaction.state == 1) {
        if (tick - g_gameReaction.waitStart >= g_gameReaction.ledOnTime) {
            g_gameReaction.state = 2;
            g_gameReaction.waitStart = tick;
            Menu_Refresh();
        }
    }
    else if (g_gameReaction.state == 2) {
        if (tick - g_gameReaction.waitStart >= 2000) {
            g_gameReaction.state = 3;
            g_gameReaction.reactionTime = 999;
            Menu_Refresh();
        }
    }
}

static void OnUpdate_Clicker(void) {
    if (g_gameClicker.active && HAL_GetTick() >= g_gameClicker.endTime) {
        g_gameClicker.active = false;
        g_gameClicker.finished = true;
        Menu_Refresh();
    }
}

// ==================== Обработчики кнопок активностей ====================
static void Clock_OnButton(Button_Type btn, bool longPress) {
    if (longPress && btn == BUTTON_SELECT) {
        Menu_OpenMenu(&g_rootMenu);
    }
}

static void Timer_OnButton(Button_Type btn, bool longPress) {
    if (longPress && btn == BUTTON_SELECT) {
        Menu_OpenMenu(&g_rootMenu);
    }
}

static void Stopwatch_OnButton(Button_Type btn, bool longPress) {
    if (longPress && btn == BUTTON_SELECT) {
        Menu_OpenMenu(&g_rootMenu);
    }
}

static void ZeroMillis_OnButton(Button_Type btn, bool longPress) {
    if (longPress && btn == BUTTON_SELECT) {
        Menu_OpenMenu(&g_rootMenu);
        return;
    }
    if (btn == BUTTON_BACK) {
        Menu_GoBack();
    } 
    
    uint32_t tick = HAL_GetTick();
    if (g_game1sec.state == 0 && btn == BUTTON_SELECT) {
        g_game1sec.state = 1;
        g_game1sec.startTime = tick;
        Menu_Refresh();
    }
    else if ((btn == BUTTON_SELECT) && (g_game1sec.state == 1)) {
        g_game1sec.state = 2;
        g_game1sec.pressTime = HAL_GetTick();
        g_game1sec.diff_ms = (g_game1sec.pressTime - g_game1sec.startTime);
        if (g_game1sec.diff_ms < 0) g_game1sec.diff_ms = -g_game1sec.diff_ms;
        Menu_Refresh();
    }
    else if ((g_game1sec.state == 2) && (btn == BUTTON_SELECT))
    {
      g_game1sec.state = 0;
    }
}

static void Reaction_OnButton(Button_Type btn, bool longPress) {
    if (longPress && btn == BUTTON_SELECT) {
        Menu_OpenMenu(&g_rootMenu);
        return;
    }
    
    if (btn == BUTTON_BACK) {
        Menu_GoBack();
    }
    
    uint32_t tick = HAL_GetTick();
    if (g_gameReaction.state == 0 && btn == BUTTON_SELECT) {
        g_gameReaction.state = 1;
        g_gameReaction.waitStart = tick;
        g_gameReaction.ledOnTime = 1000 + (rand() % 4000);
        Menu_Refresh();
    }
    else if (g_gameReaction.state == 2 && btn == BUTTON_SELECT) {
        g_gameReaction.reactionTime = tick - g_gameReaction.waitStart;
        g_gameReaction.state = 3;
        Menu_Refresh();
    }
    else if ((g_gameReaction.state == 3) && (btn == BUTTON_SELECT))
    {
      g_gameReaction.state = 0;
    }
}

static void Clicker_OnButton(Button_Type btn, bool longPress) {
    if (longPress && btn == BUTTON_SELECT) {
        Menu_OpenMenu(&g_rootMenu);
        return;
    }
    
    if (btn == BUTTON_BACK) {
        Menu_GoBack();
    }
    
    if (!g_gameClicker.active && !g_gameClicker.finished && btn == BUTTON_SELECT) {
        g_gameClicker.active = true;
        g_gameClicker.endTime = HAL_GetTick() + 10000;
        g_gameClicker.clicks = 0;
        Menu_Refresh();
    }
    else if (g_gameClicker.active && btn == BUTTON_UP) {
        g_gameClicker.clicks++;
        Menu_Refresh();
    }
    else if (g_gameClicker.finished && (btn == BUTTON_SELECT))
    {
      g_gameClicker.finished = 0;
      g_gameClicker.active = 0;
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

// ==================== Узлы активностей (корень) ====================
MenuNode g_clockNode = {
    .name = "CLOC",
    .parent = NULL,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .display = Display_Clock,
    .onEnter = OnEnter_Clock,
    .onUpdate = NULL,
    .onButton = Clock_OnButton,
    .data = NULL
};

MenuNode g_timerNode = {
    .name = "TIMER",
    .parent = NULL,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .display = Display_Timer,
    .onEnter = OnEnter_Timer,
    .onUpdate = NULL,
    .onButton = Timer_OnButton,
    .data = NULL
};

MenuNode g_stopwatchNode = {
    .name = "SECOND",
    .parent = NULL,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .display = Display_Stopwatch,
    .onEnter = OnEnter_Stopwatch,
    .onUpdate = NULL,
    .onButton = Stopwatch_OnButton,
    .data = NULL
};

MenuNode g_ZeroMillisNode = {
    .name = "00 SEC",
    .parent = &g_gamesNode,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .needsRedraw = 1,
    .display = Display_ZeroMillis,
    .onEnter = OnEnter_ZeroMillis,
    .onUpdate = NULL,
    .onButton = ZeroMillis_OnButton,
    .data = &g_game1sec
};

MenuNode g_ReactionTimeNode = {
    .name = "CSTEST",
    .parent = &g_gamesNode,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .display = Display_Reaction,
    .onEnter = OnEnter_Reaction,
    .onUpdate = OnUpdate_Reaction,
    .onButton = Reaction_OnButton,
    .data = &g_gameReaction
};

MenuNode g_ClickerTimeNode = {
    .name = "CLICER",
    .parent = &g_gamesNode,
    .children = NULL,
    .childCount = 0,
    .selectedChild = 0,
    .display = Display_Clicker,
    .onEnter = OnEnter_Clicker,
    .onUpdate = OnUpdate_Clicker,
    .onButton = Clicker_OnButton,
    .data = &g_gameClicker
};

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

static MenuNode* g_gamesChildren[] = {
    &g_ZeroMillisNode,
    &g_ReactionTimeNode,
    &g_ClickerTimeNode
};

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

static MenuNode g_rootMenu = {
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
        .childCount = 3,
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
        .childCount = 7,
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