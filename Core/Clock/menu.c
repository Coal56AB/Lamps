#include "menu.h"
#include "segment.h"
#include "clock_manager.h"

// Время удержания кнопки (мс), после которого нажатие считается "длинным"
// Используется для входа в меню из режима часов
#define LONG_PRESS_MS       500

// Задержка перед началом автоповтора (мс)
// При удержании кнопки первые 500 мс ничего не происходит
#define REPEAT_DELAY_MS     500

// Интервал между автоповторами (мс)
// После начала повтора каждые 100 мс кнопка срабатывает заново
#define REPEAT_INTERVAL_MS  100

// Интервал мигания редактируемого разряда (мс)
// Моргает каждые 500 мс (0.5 сек вкл / 0.5 сек выкл)
#define BLINK_INTERVAL_MS   500

// Задержка антидребезга (мс)
// Изменение состояния кнопки фиксируется только если оно стабильно в течение 30 мс
#define DEBOUNCE_MS         30

typedef enum {
    MAIN_MENU_SET_TIME,
    MAIN_MENU_SET_DUTY,
    MAIN_MENU_RESET,
    MAIN_MENU_COUNT
} MainMenuItem;

typedef struct {
    // Состояние
    SystemState state;
    MainMenuItem selectedMenuItem;
    
    // Для SET_TIME
    time_t originalTime;
    time_t editTime;
    uint8_t editStep;
    bool blinkState;
    uint32_t lastBlinkTime;
    
    // Для SET_DUTY
    uint8_t originalDuty;
    uint8_t editDuty;
    
    // Для кнопок
    bool buttonPrevState[BUTTON_COUNT];
    uint32_t buttonPressTime[BUTTON_COUNT];
    bool longPressSent[BUTTON_COUNT];
    bool repeatActive[BUTTON_COUNT];
    uint32_t lastRepeatTime[BUTTON_COUNT];
    uint32_t lastDebounceTime[BUTTON_COUNT];
    bool buttonStableState[BUTTON_COUNT];
    
} MenuContext;

MenuContext menu;

// ==================== Отображение ====================
static void FormatTime(char* buf, const time_t* t) {
    buf[0] = '0' + t->hour / 10;
    buf[1] = '0' + t->hour % 10;
    buf[2] = '0' + t->min / 10;
    buf[3] = '0' + t->min % 10;
    buf[4] = '0' + t->sec / 10;
    buf[5] = '0' + t->sec % 10;
}

static void UpdateDisplay(void) {
    switch (menu.state) {
        case STATE_CLOCK: {
            time_t now = ClockManager_GetTime();
            char buf[7];
            FormatTime(buf, &now);
            Segment_SetString(buf);
            break;
        }
        
        case STATE_MAIN_MENU: {
            switch (menu.selectedMenuItem) {
                case MAIN_MENU_SET_TIME:
                    Segment_SetString("SET T ");
                    break;
                case MAIN_MENU_SET_DUTY:
                    Segment_SetString("SET D ");
                    break;
                case MAIN_MENU_RESET:
                    Segment_SetString("RESET ");
                    break;
                default:
                    break;
            }
            break;
        }
        
        case STATE_SET_TIME: {
            char buf[7];
            FormatTime(buf, &menu.editTime);
            if (menu.blinkState && menu.editStep < 6) {
                buf[menu.editStep] = ' ';
            }
            Segment_SetString(buf);
            break;
        }
        
        case STATE_SET_DUTY: {
            char buf[6] = {'D', 'U', 'T', 'Y', ' ' , ' '};
            if (menu.editDuty == 10) {
                buf[4] = '1';
                buf[5] = '0';
            } else {
                buf[5] = '0' + menu.editDuty;
            }
            Segment_SetString(buf);
            break;
        }
        
        case STATE_RESET_CONFIRM:
            Segment_SetString("RESET ");
            break;
    }
}

// ==================== Логика SET_TIME ====================
static void IncreaseTimeDigit(void) {
    uint8_t tens, units;
    switch (menu.editStep) {
        case 0: // десятки часов (0-2)
            tens = menu.editTime.hour / 10;
            units = menu.editTime.hour % 10;
            tens++;
            if (tens > 2) tens = 0;
            menu.editTime.hour = tens * 10 + units;
            break;
            
        case 1: // единицы часов (0-9, но с учетом десятков)
            tens = menu.editTime.hour / 10;
            units = menu.editTime.hour % 10;
            units++;
            if (tens == 2 && units > 3) units = 0;  // 23 → 20
            if (units > 9) units = 0;
            menu.editTime.hour = tens * 10 + units;
            break;
            
        case 2: // десятки минут (0-5)
            tens = menu.editTime.min / 10;
            units = menu.editTime.min % 10;
            tens = (tens + 1) % 6;
            menu.editTime.min = tens * 10 + units;
            break;
            
        case 3: // единицы минут (0-9)
            units = (menu.editTime.min % 10 + 1) % 10;
            menu.editTime.min = (menu.editTime.min / 10) * 10 + units;
            break;
            
        case 4: // десятки секунд (0-5)
            tens = menu.editTime.sec / 10;
            units = menu.editTime.sec % 10;
            tens = (tens + 1) % 6;
            menu.editTime.sec = tens * 10 + units;
            break;
            
        case 5: // единицы секунд (0-9)
            units = (menu.editTime.sec % 10 + 1) % 10;
            menu.editTime.sec = (menu.editTime.sec / 10) * 10 + units;
            break;
    }
    UpdateDisplay();
}

static void DecreaseTimeDigit(void) {
    uint8_t tens, units;
    switch (menu.editStep) {
        case 0: // десятки часов (0-2)
            tens = menu.editTime.hour / 10;
            units = menu.editTime.hour % 10;
            if (tens == 0) tens = 2;
            else tens--;
            menu.editTime.hour = tens * 10 + units;
            break;
            
        case 1: // единицы часов
            tens = menu.editTime.hour / 10;
            units = menu.editTime.hour % 10;
            if (units == 0) {
                units = (tens == 2) ? 3 : 9;
            } else {
                units--;
            }
            menu.editTime.hour = tens * 10 + units;
            break;
            
        case 2: // десятки минут (0-5)
            tens = menu.editTime.min / 10;
            units = menu.editTime.min % 10;
            tens = (tens == 0) ? 5 : tens - 1;
            menu.editTime.min = tens * 10 + units;
            break;
            
        case 3: // единицы минут
            units = (menu.editTime.min % 10 == 0) ? 9 : (menu.editTime.min % 10) - 1;
            menu.editTime.min = (menu.editTime.min / 10) * 10 + units;
            break;
            
        case 4: // десятки секунд (0-5)
            tens = menu.editTime.sec / 10;
            units = menu.editTime.sec % 10;
            tens = (tens == 0) ? 5 : tens - 1;
            menu.editTime.sec = tens * 10 + units;
            break;
            
        case 5: // единицы секунд
            units = (menu.editTime.sec % 10 == 0) ? 9 : (menu.editTime.sec % 10) - 1;
            menu.editTime.sec = (menu.editTime.sec / 10) * 10 + units;
            break;
    }
    UpdateDisplay();
}

// ==================== Обработка кнопок ====================
static void ProcessButton(Button_Type btn, bool longPress) {
    // Длинное нажатие SELECT в режиме часов - вход в меню
    if (menu.state == STATE_CLOCK && longPress && btn == BUTTON_SELECT) {
        menu.state = STATE_MAIN_MENU;
        menu.selectedMenuItem = MAIN_MENU_SET_TIME;
        UpdateDisplay();
        return;
    }
    
    switch (menu.state) {
        case STATE_MAIN_MENU:
            if (btn == BUTTON_UP) {
                menu.selectedMenuItem++;
                if (menu.selectedMenuItem >= MAIN_MENU_COUNT) {
                    menu.selectedMenuItem = 0;
                }
                UpdateDisplay();
            }
            else if (btn == BUTTON_DOWN) {
                if (menu.selectedMenuItem == 0) {
                    menu.selectedMenuItem = MAIN_MENU_COUNT - 1;
                } else {
                    menu.selectedMenuItem--;
                }
                UpdateDisplay();
            }
            else if (btn == BUTTON_SELECT && !longPress) {
                switch (menu.selectedMenuItem) {
                    case MAIN_MENU_SET_TIME:
                        menu.state = STATE_SET_TIME;
                        menu.originalTime = ClockManager_GetTime();
                        menu.editTime = menu.originalTime;
                        menu.editStep = 0;
                        menu.blinkState = true;
                        menu.lastBlinkTime = HAL_GetTick();
                        break;
                    case MAIN_MENU_SET_DUTY:
                        menu.state = STATE_SET_DUTY;
                        menu.originalDuty = ClockManager_GetDuty();
                        menu.editDuty = menu.originalDuty;
                        break;
                    case MAIN_MENU_RESET:
                        menu.state = STATE_RESET_CONFIRM;
                        break;
                    default: break;
                }
                UpdateDisplay();
            }
            else if (btn == BUTTON_BACK) {
                menu.state = STATE_CLOCK;
                UpdateDisplay();
            }
            break;
            
        case STATE_SET_TIME:
            if (btn == BUTTON_UP) {
                IncreaseTimeDigit();
            }
            else if (btn == BUTTON_DOWN) {
                DecreaseTimeDigit();
            }
            else if (btn == BUTTON_SELECT) {
                menu.editStep++;
                if (menu.editStep >= 6) {
                    ClockManager_SetTime(menu.editTime.hour, menu.editTime.min, menu.editTime.sec);
                    menu.state = STATE_CLOCK;
                }
                UpdateDisplay();
            }
            else if (btn == BUTTON_BACK) {
                menu.state = STATE_CLOCK;
                UpdateDisplay();
            }
            break;
            
        case STATE_SET_DUTY:
            if (btn == BUTTON_UP && menu.editDuty < 10) {
                menu.editDuty++;
                Segment_SetBrightness(menu.editDuty * 10);
                UpdateDisplay();
            }
            else if (btn == BUTTON_DOWN && menu.editDuty > 0) {
                menu.editDuty--;
                Segment_SetBrightness(menu.editDuty * 10);
                UpdateDisplay();
            }
            else if (btn == BUTTON_SELECT) {
                ClockManager_SetDuty(menu.editDuty);
                menu.state = STATE_CLOCK;
                UpdateDisplay();
            }
            else if (btn == BUTTON_BACK) {
                menu.state = STATE_CLOCK;
                Segment_SetBrightness(menu.originalDuty * 10);
                UpdateDisplay();
            }
            break;
            
        case STATE_RESET_CONFIRM:
            if (btn == BUTTON_SELECT) {
                ClockManager_ResetTime();
                ClockManager_SetDuty(8);
                menu.state = STATE_CLOCK;
                UpdateDisplay();
            }
            else if (btn == BUTTON_BACK) {
                menu.state = STATE_CLOCK;
                UpdateDisplay();
            }
            break;
            
        default:
            break;
    }
}

// ==================== Публичные функции ====================
void Menu_Init(void) {
    menu.state = STATE_CLOCK;
    menu.selectedMenuItem = MAIN_MENU_SET_TIME;
    menu.editStep = 0;
    menu.blinkState = false;
    menu.lastBlinkTime = 0;
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        menu.buttonPrevState[i] = false;
        menu.buttonPressTime[i] = 0;
        menu.longPressSent[i] = false;
        menu.repeatActive[i] = false;
        menu.lastDebounceTime[i] = 0;
        menu.buttonStableState[i] = false;
    }
}

void Menu_Process(void) {
    uint32_t tick = HAL_GetTick();
    
    if (menu.state == STATE_SET_TIME) {
        if (tick - menu.lastBlinkTime >= BLINK_INTERVAL_MS) {
            menu.lastBlinkTime = tick;
            menu.blinkState = !menu.blinkState;
            UpdateDisplay();
        }
    }
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        bool rawPressed = ReadButton(i);
        bool pressed;

        // Антидребезг
        if (rawPressed != menu.buttonStableState[i]) {
            menu.lastDebounceTime[i] = tick;
            menu.buttonStableState[i] = rawPressed;
        }

        if ((tick - menu.lastDebounceTime[i]) >= DEBOUNCE_MS) {
            pressed = menu.buttonStableState[i];
        } else {
            pressed = menu.buttonPrevState[i]; // старое значение пока не стабилизировалось
        }
        bool wasPressed = menu.buttonPrevState[i];
        
        if (pressed && !wasPressed) {
            menu.buttonPressTime[i] = tick;
            menu.longPressSent[i] = false;
            menu.repeatActive[i] = false;
        }
        else if (pressed && wasPressed) {
            if (!menu.longPressSent[i] && (tick - menu.buttonPressTime[i] >= LONG_PRESS_MS)) {
                menu.longPressSent[i] = true;
                ProcessButton((Button_Type)i, true);
            }
            else if (menu.longPressSent[i] && !menu.repeatActive[i] && 
                     (tick - menu.buttonPressTime[i] >= REPEAT_DELAY_MS)) {
                menu.repeatActive[i] = true;
                menu.lastRepeatTime[i] = tick;
                ProcessButton((Button_Type)i, true);
            }
            else if (menu.repeatActive[i] && (tick - menu.lastRepeatTime[i] >= REPEAT_INTERVAL_MS)) {
                menu.lastRepeatTime[i] = tick;
                ProcessButton((Button_Type)i, true);
            }
        }
        else if (!pressed && wasPressed) {
            if (!menu.longPressSent[i]) {
                ProcessButton((Button_Type)i, false);
            }
            menu.repeatActive[i] = false;
        }
        
        menu.buttonPrevState[i] = pressed;
    }
    
    static uint32_t lastClockUpdate = 0;
    if (menu.state == STATE_CLOCK && (tick - lastClockUpdate >= 200)) {
        lastClockUpdate = tick;
        UpdateDisplay();
    }
}

SystemState Menu_GetState(void) {
    return menu.state;
}