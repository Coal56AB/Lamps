#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "melody.h"

// Макросы звуковых эффектов
#define SFX_BPM 480
#define SOUND_CLICK     if(g_sound_enabled) Melody_Play(&melody, &SFX_Click, SFX_BPM)
#define SOUND_DOUBLE    if(g_sound_enabled) Melody_Play(&melody, &SFX_DoubleBeep, SFX_BPM)
#define SOUND_ERROR     if(g_sound_enabled) Melody_Play(&melody, &SFX_Error, SFX_BPM)
#define SOUND_SUCCESS   if(g_sound_enabled) Melody_Play(&melody, &SFX_Success, SFX_BPM)
  
typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_SELECT,
    BUTTON_BACK,
    BUTTON_COUNT
} Button_Type;

typedef struct MenuNode {
    const char* name;
    struct MenuNode* parent;
    struct MenuNode** children;
    uint8_t childCount;
    uint8_t selectedChild;
    uint8_t needsRedraw;
    
    void (*display)(void);
    void (*onEnter)(void);
    void (*onUpdate)(void);
    void (*onButton)(Button_Type btn, bool longPress);
    
    void* data;
} MenuNode;

typedef struct {
    MenuNode* currentNode;
    uint32_t lastTick;
    bool needsRedraw;
  
  
    uint32_t  lastDebounceTime[BUTTON_COUNT];
    uint8_t   lastButtonState[BUTTON_COUNT];
    uint8_t   buttonState[BUTTON_COUNT];
    uint32_t  pressStartTime[BUTTON_COUNT];
    uint8_t   longPressTriggered[BUTTON_COUNT];
} MenuContext;

// Глобальные переменные
extern uint8_t g_sound_enabled;
extern MenuContext g_ctx;

// Функции
void Menu_Init(MenuNode* startNode);
void Menu_Process(void);
void Menu_HandleButton(Button_Type btn, bool longPress);
void Menu_GoBack(void);
void Menu_Refresh(void);
MenuNode* Menu_GetCurrentNode(void);
void Menu_OpenMenu(MenuNode* menu);
void Menu_Sound_On(void);
void Menu_Sound_Off(void);
void Menu_Sound_Toggle(void);
uint8_t Menu_Sound_IsEnabled(void);

#endif