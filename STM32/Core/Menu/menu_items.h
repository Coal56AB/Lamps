#ifndef MENU_ITEMS_H
#define MENU_ITEMS_H

#include "menu.h"

extern MenuNode* g_currentActivity;
// Глобальные узлы меню (для доступа из main.c)
extern MenuNode g_rootMenu;
extern MenuNode g_gamesNode;
extern MenuNode g_settingsNode;

// Инициализация всех пунктов меню
void MenuItems_Init(void);

#endif
