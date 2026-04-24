#include "menu_items.h"
#include "games.h"
#include "segment.h"
#include "clock_manager.h"
#include "menu_items.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


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

static Game1SecData g_game1sec;
static GameReactionData g_gameReaction;
static GameClickerData g_gameClicker;


/////// ZERO MILLIS ////////
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

static void OnEnter_ZeroMillis(void) {
    g_game1sec.state = 0;
    Menu_Refresh();
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



/////// REACTION ////////
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

static void Display_Reaction(void) {
    char buf[7];
    switch (g_gameReaction.state) {
        case 0: sprintf(buf, "START"); break;
        case 1: sprintf(buf, "      "); break;
        case 2: sprintf(buf, "888888"); break;
        case 3: sprintf(buf, "%6d", g_gameReaction.reactionTime); break;
        case 4: sprintf(buf, "FAIL"); break;
        default: sprintf(buf, "ERROR"); break;
    }
    Segment_SetString(buf);
}


static void OnEnter_Reaction(void) {
    g_gameReaction.state = 0;
    g_ctx.debounceTime = 0;
    Menu_Refresh();
}

static void Reaction_OnButton(Button_Type btn, bool longPress) {
    if (longPress && btn == BUTTON_SELECT) {
        Menu_OpenMenu(&g_rootMenu);
        return;
    }
    
    if (btn == BUTTON_BACK) {
        Menu_GoBack();
        g_ctx.debounceTime = 30;
    }
    
    uint32_t tick = HAL_GetTick();
    if (g_gameReaction.state == 0 && btn == BUTTON_SELECT) {
        g_gameReaction.state = 1;
        g_gameReaction.waitStart = tick;
        g_gameReaction.ledOnTime = 1200 + (rand() % 2000);
        g_ctx.debounceTime = 0;
        Menu_Refresh();
    }
    else if (g_gameReaction.state == 1 && btn == BUTTON_SELECT) {
        g_gameReaction.reactionTime = 0;
        g_gameReaction.state = 4;
        g_ctx.debounceTime = 30;
        Menu_Refresh();
    }
    else if (g_gameReaction.state == 2 && btn == BUTTON_SELECT) {
        g_gameReaction.reactionTime = tick - g_gameReaction.waitStart;
        g_gameReaction.state = 3;
        g_ctx.debounceTime = 30;
        Menu_Refresh();
    }
    else if ((g_gameReaction.state >= 3) && (btn == BUTTON_SELECT))
    {
      g_gameReaction.state = 1;
      g_gameReaction.waitStart = tick;
      g_gameReaction.ledOnTime = 500 + (rand() % 2000);
      g_ctx.debounceTime = 0;
    }
}



/////// CLICLER ////////
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

static void OnEnter_Clicker(void) {
    g_gameClicker.active = false;
    g_gameClicker.finished = false;
    g_gameClicker.clicks = 0;
    Menu_Refresh();
}

static void OnUpdate_Clicker(void) {
    if (g_gameClicker.active && HAL_GetTick() >= g_gameClicker.endTime) {
        g_gameClicker.active = false;
        g_gameClicker.finished = true;
        Menu_Refresh();
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

// NODES
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

MenuNode* g_gamesChildren[3] = {
    &g_ZeroMillisNode,
    &g_ReactionTimeNode,
    &g_ClickerTimeNode
};
