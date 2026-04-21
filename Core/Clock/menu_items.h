#ifndef MENU_ITEMS_H
#define MENU_ITEMS_H

#include "menu.h"

// Глобальные узлы меню (для доступа из main.c)
extern MenuNode g_clockNode;
extern MenuNode g_timerNode;
extern MenuNode g_stopwatchNode;
extern MenuNode g_gamesNode;
extern MenuNode g_settingsNode;
extern MenuNode g_timeEditNode;
extern MenuNode g_dutyEditNode;
extern MenuNode g_resetNode;

// Инициализация всех пунктов меню
void MenuItems_Init(void);

#endif