#include "menu.h"
#include "segment.h"
#include <string.h>
#include <stdio.h>

#define LONG_PRESS_MS   500

MenuContext g_ctx;
uint8_t g_sound_enabled = 1;

extern bool ReadButton(Button_Type btn);
extern MenuNode* Menu_GetRootMenu(void);

void Menu_Init(MenuNode* startNode) {
    g_ctx.currentNode = startNode;
    g_ctx.needsRedraw = true;
    g_ctx.lastTick = 0;
    g_ctx.debounceTime = 30;
    if (startNode && startNode->onEnter) {
        startNode->onEnter();
    }
}

// НОВАЯ ФУНКЦИЯ - открыть меню
void Menu_OpenMenu(MenuNode* menu) {
    if (!menu) return;
    g_ctx.currentNode = menu;
    g_ctx.needsRedraw = true;
    if (menu->onEnter) {
        menu->onEnter();
    }
}

void Menu_HandleButton(Button_Type btn, bool longPress) {
    if (!g_ctx.currentNode) return;
    
    // Длинное нажатие SELECT на корневом уровне (часы) - вход в меню
    if (longPress && btn == BUTTON_SELECT && g_ctx.currentNode->parent == NULL) {
        SOUND_DOUBLE;
        MenuNode* rootMenu = Menu_GetRootMenu();
        if (rootMenu) {
            Menu_OpenMenu(rootMenu);
        }
        return;
    }
    
    if (g_ctx.currentNode->onButton) {
        if(btn == BUTTON_SELECT)
        {
          SOUND_DOUBLE;
        }
        else
        {
          SOUND_CLICK;
        }
        
        g_ctx.currentNode->onButton(btn, longPress);
        return;
    }
    
    if (g_ctx.currentNode->parent != NULL) {
        switch (btn) {
            case BUTTON_UP:
                SOUND_CLICK;
                g_ctx.currentNode->selectedChild--;
                if (g_ctx.currentNode->selectedChild > g_ctx.currentNode->childCount - 1) {
                  g_ctx.currentNode->selectedChild = g_ctx.currentNode->childCount - 1;
                }
                g_ctx.needsRedraw = true;
                break;
                
            case BUTTON_DOWN:
                SOUND_CLICK;
                g_ctx.currentNode->selectedChild++;
                if (g_ctx.currentNode->selectedChild > g_ctx.currentNode->childCount - 1) {
                g_ctx.currentNode->selectedChild = 0;
                }
                g_ctx.needsRedraw = true;
                break;
                
            case BUTTON_SELECT: {
                SOUND_DOUBLE;
                if (g_ctx.currentNode->children) {
                    MenuNode* selected = g_ctx.currentNode->children[g_ctx.currentNode->selectedChild];
                    if (selected) {
                        g_ctx.currentNode = selected;
                        g_ctx.needsRedraw = true;
                        if (selected->onEnter) {
                            selected->onEnter();
                        }
                    }
                }
                break;
            }
                
            case BUTTON_BACK:
                SOUND_CLICK;
                Menu_GoBack();
                break;
            
            default:
              break;
        }
    }
}

void Menu_Process(void) {
    uint32_t tick = HAL_GetTick();
    
    // ОБРАБОТКА КНОПОК С ДЕБАУНСОМ
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        bool reading = ReadButton((Button_Type)i);
        
        if (reading != g_ctx.lastButtonState[i]) {
            g_ctx.lastDebounceTime[i] = tick;
        }
        
        if ((tick - g_ctx.lastDebounceTime[i]) >= g_ctx.debounceTime) {  // DEBOUNCE_MS = 30
            if (reading != g_ctx.buttonState[i]) {
                g_ctx.buttonState[i] = reading;
                
                if (g_ctx.buttonState[i]) {
                    g_ctx.pressStartTime[i] = tick;
                    g_ctx.longPressTriggered[i] = false;
                } else {
                    if (!g_ctx.longPressTriggered[i]) {
                        Menu_HandleButton((Button_Type)i, false);
                    }
                }
            }
        }
        
        if (g_ctx.buttonState[i] && !g_ctx.longPressTriggered[i]) {
            if ((tick - g_ctx.pressStartTime[i]) >= LONG_PRESS_MS) {
                g_ctx.longPressTriggered[i] = true;
                Menu_HandleButton((Button_Type)i, true);
            }
        }
        
        g_ctx.lastButtonState[i] = reading;
    }
    
    if (g_ctx.currentNode && g_ctx.currentNode->onUpdate) {
        g_ctx.currentNode->onUpdate();
    }
    
    if (tick - g_ctx.lastTick >= 200) {
        g_ctx.lastTick = tick;
        g_ctx.needsRedraw = true;
    }
    
    if ((g_ctx.needsRedraw && g_ctx.currentNode && g_ctx.currentNode->display) ||
      (g_ctx.currentNode && g_ctx.currentNode->display && g_ctx.currentNode->needsRedraw)) {
        g_ctx.currentNode->display();
        g_ctx.needsRedraw = false;
    }
}

void Menu_GoBack(void) {
    if (g_ctx.currentNode && g_ctx.currentNode->parent) {
        g_ctx.currentNode = g_ctx.currentNode->parent;
        g_ctx.needsRedraw = true;
        if (g_ctx.currentNode->onEnter) {
            g_ctx.currentNode->onEnter();
        }
    }
}

void Menu_Refresh(void) {
    g_ctx.needsRedraw = true;
}

MenuNode* Menu_GetCurrentNode(void) {
    return g_ctx.currentNode;
}

void Menu_Sound_On(void) {
    g_sound_enabled = 1;
    SOUND_SUCCESS;
}

void Menu_Sound_Off(void) {
    g_sound_enabled = 0;
}

void Menu_Sound_Toggle(void) {
    if (g_sound_enabled) {
        Menu_Sound_Off();
    } else {
        Menu_Sound_On();
    }
}

uint8_t Menu_Sound_IsEnabled(void) {
    return g_sound_enabled;
}
