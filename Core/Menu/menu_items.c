#include "menu_items.h"
#include "segment.h"
#include "clock_manager.h"
#include "games.h"
#include "clock.h"
#include "settings.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
MenuNode g_rootMenu;
// объявляем узлы меню
MenuNode g_gamesNode;
MenuNode g_settingsNode;


// ==================== Данные ====================

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

// ==================== Узлы меню ====================
MenuNode g_gamesNode;
MenuNode g_settingsNode;

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
}

MenuNode* Menu_GetRootMenu(void) {
    return &g_rootMenu;
}

MenuNode* Menu_GetCurrentActivity(void) {
    return g_currentActivity;
}
