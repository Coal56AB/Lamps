#include "segment.h"
#include "main.h"

// ==================== Конфигурация ====================
#define DIGITS_COUNT 6          // Количество разрядов
#define PROCESS_INTERVAL_US 10  // Интервал вызова Segment_Process в микросекундах

volatile uint32_t REFRESH_RATE = 80;         // Частота обновления всех цифр в Гц (можно менять)
volatile uint8_t GLOBAL_BRIGHTNESS = 100;      // Глобальная яркость в процентах (0-100)

// Маска переназначения: новый_бит = старый_бит
// Так как перепутаны бит5 (F) и бит6 (G), меняем их местами
#define SWAP_BIT5_BIT6(x) (((x) & 0x9F) | (((x) & 0x20) << 1) | (((x) & 0x40) >> 1))

// Макросы для быстрой работы с пинами (вместо HAL_GPIO_WritePin)
#define SET_SEGMENT_A(val) SEGMENT_A_GPIO_Port->BSRR = ((uint32_t)SEGMENT_A_Pin << 16) | ((val) ? 0 : SEGMENT_A_Pin)
#define SET_SEGMENT_B(val) SEGMENT_B_GPIO_Port->BSRR = ((uint32_t)SEGMENT_B_Pin << 16) | ((val) ? 0 : SEGMENT_B_Pin)
#define SET_SEGMENT_C(val) SEGMENT_C_GPIO_Port->BSRR = ((uint32_t)SEGMENT_C_Pin << 16) | ((val) ? 0 : SEGMENT_C_Pin)
#define SET_SEGMENT_D(val) SEGMENT_D_GPIO_Port->BSRR = ((uint32_t)SEGMENT_D_Pin << 16) | ((val) ? 0 : SEGMENT_D_Pin)
#define SET_SEGMENT_E(val) SEGMENT_E_GPIO_Port->BSRR = ((uint32_t)SEGMENT_E_Pin << 16) | ((val) ? 0 : SEGMENT_E_Pin)
#define SET_SEGMENT_F(val) SEGMENT_F_GPIO_Port->BSRR = ((uint32_t)SEGMENT_F_Pin << 16) | ((val) ? 0 : SEGMENT_F_Pin)
#define SET_SEGMENT_G(val) SEGMENT_G_GPIO_Port->BSRR = ((uint32_t)SEGMENT_G_Pin << 16) | ((val) ? 0 : SEGMENT_G_Pin)

// ==================== Таблица символов ====================
// Для общего анода: 0 - сегмент горит, 1 - сегмент не горит
// Биты: A B C D E F G
typedef struct {
  char c;
  uint8_t mask;
} CharMap;

static const CharMap charTable[] = {
    // ==================== Цифры ====================
    {'0', 0xC0},
    {'1', 0xF9},
    {'2', 0xA4},
    {'3', 0xB0},
    {'4', 0x99},
    {'5', 0x92},
    {'6', 0x82},
    {'7', 0xF8},
    {'8', 0x80},
    {'9', 0x90},

    // ==================== Буквы ====================
    {'A', 0x88}, {'a', 0x88},
    {'B', 0x83}, {'b', 0x83},
    {'C', 0xC6}, {'c', 0xC6},
    {'D', 0xA1}, {'d', 0xA1},
    {'E', 0x86}, {'e', 0x86},
    {'F', 0x8E}, {'f', 0x8E},
    {'G', 0xC2}, {'g', 0xC2},
    {'H', 0x89}, {'h', 0x89},
    {'I', 0xF9}, {'i', 0xF9},
    {'J', 0xF1}, {'j', 0xF1},
    {'L', 0xC7}, {'l', 0xC7},
    {'N', 0xAB}, {'n', 0xAB},
    {'O', 0xC0}, {'o', 0xC0},
    {'P', 0x8C}, {'p', 0x8C},
    {'Q', 0x98}, {'q', 0x98},
    {'R', 0xAF}, {'r', 0xAF},
    {'S', 0x92}, {'s', 0x92},
    {'T', 0x87}, {'t', 0x87},
    {'U', 0xC1}, {'u', 0xC1},
    {'Y', 0x91}, {'y', 0x91},

    // ==================== Символы ====================
    {'-', 0xBF},
    {'_', 0xF7},
    {' ', 0xFF}
};

#define CHAR_TABLE_SIZE (sizeof(charTable)/sizeof(CharMap))

// ==================== Статические переменные ====================
static uint8_t displayBuffer[DIGITS_COUNT];  // Буфер СРАЗУ МАСОК сегментов
static uint8_t currentPos;

static uint32_t pwmCounter;
static uint32_t currentDigitTime;
static uint32_t currentPwmThreshold;

// ==================== Низкий уровень ====================
static void SetSegments(uint8_t mask) {
  SET_SEGMENT_A((mask >> 0) & 1);
  SET_SEGMENT_B((mask >> 1) & 1);
  SET_SEGMENT_C((mask >> 2) & 1);
  SET_SEGMENT_D((mask >> 3) & 1);
  SET_SEGMENT_E((mask >> 4) & 1);
  SET_SEGMENT_F((mask >> 5) & 1);
  SET_SEGMENT_G((mask >> 6) & 1);
}

// Получить маску по символу
static uint8_t GetCharMask(char c) {
  for (int i = 0; i < CHAR_TABLE_SIZE; i++) {
    if (charTable[i].c == c)
      return charTable[i].mask;
  }
  return 0xFF; // если символ неизвестен — пусто
}

// ==================== Управление разрядами ====================
void DisableAllDigits(void) {
  SET_SEGMENT_A(1); SET_SEGMENT_B(1); SET_SEGMENT_C(1);
  SET_SEGMENT_D(1); SET_SEGMENT_E(1); SET_SEGMENT_F(1);
  SET_SEGMENT_G(1);

  DIGIT_HOUR_H_GPIO_Port->BSRR = DIGIT_HOUR_H_Pin << 16;
  DIGIT_HOUR_L_GPIO_Port->BSRR = DIGIT_HOUR_L_Pin << 16;
  DIGIT_MIN_H_GPIO_Port->BSRR = DIGIT_MIN_H_Pin << 16;
  DIGIT_MIN_L_GPIO_Port->BSRR = DIGIT_MIN_L_Pin << 16;
  DIGIT_SEC_H_GPIO_Port->BSRR = DIGIT_SEC_H_Pin << 16;
  DIGIT_SEC_L_GPIO_Port->BSRR = DIGIT_SEC_L_Pin << 16;
}

void EnableDigit(uint8_t pos) {
  switch(pos) {
    case 0: DIGIT_HOUR_H_GPIO_Port->BSRR = DIGIT_HOUR_H_Pin; break;
    case 1: DIGIT_HOUR_L_GPIO_Port->BSRR = DIGIT_HOUR_L_Pin; break;
    case 2: DIGIT_MIN_H_GPIO_Port->BSRR = DIGIT_MIN_H_Pin; break;
    case 3: DIGIT_MIN_L_GPIO_Port->BSRR = DIGIT_MIN_L_Pin; break;
    case 4: DIGIT_SEC_H_GPIO_Port->BSRR = DIGIT_SEC_H_Pin; break;
    case 5: DIGIT_SEC_L_GPIO_Port->BSRR = DIGIT_SEC_L_Pin; break;
  }
}

// ==================== Логика ====================
static void NextDigit(void) {
  currentPos++;
  if (currentPos >= DIGITS_COUNT) currentPos = 0;

  if (REFRESH_RATE == 0) REFRESH_RATE = 1;

  uint32_t totalCalls = (1000000UL / REFRESH_RATE) / PROCESS_INTERVAL_US;
  if (totalCalls < DIGITS_COUNT) totalCalls = DIGITS_COUNT;

  currentDigitTime = totalCalls / DIGITS_COUNT;
  if (currentDigitTime < 1) currentDigitTime = 1;

  currentPwmThreshold = (currentDigitTime * GLOBAL_BRIGHTNESS) / 100;

  pwmCounter = 0;
}

static void UpdateOneDigit(void) {
  if (pwmCounter == 0) {
    DisableAllDigits();

    __NOP(); __NOP(); // анти-гостинг

    uint8_t mask = displayBuffer[currentPos];

    // SWAP только для позиции 5
    if (currentPos == 5)
      mask = SWAP_BIT5_BIT6(mask);

    SetSegments(mask);
    EnableDigit(currentPos);
   }

  if (pwmCounter >= currentPwmThreshold) {
    DisableAllDigits();
    SetSegments(0xFF);
  }

  pwmCounter++;

  if (pwmCounter >= currentDigitTime) {
    NextDigit();
  }
}

// ==================== Публичные функции ====================

// Инициализация модуля
void Segment_Init(void) {
  currentPos = 0;
  pwmCounter = 0;
  currentDigitTime = 1;
  currentPwmThreshold = 1;

  for (int i = 0; i < DIGITS_COUNT; i++)
    displayBuffer[i] = 0xFF; // пусто

  NextDigit();
}

// Установить символ в разряд
void Segment_SetChar(uint8_t pos, char c) {
  if (pos >= DIGITS_COUNT) return;
  displayBuffer[pos] = GetCharMask(c);
}

// Установить напрямую маску сегментов
void Segment_SetRaw(uint8_t pos, uint8_t mask) {
  if (pos >= DIGITS_COUNT) return;
  displayBuffer[pos] = mask;
}

// Установить строку (например "HELLO ")
void Segment_SetString(const char *str) {
  for (int i = 0; i < DIGITS_COUNT; i++) {
    if (str[i] == 0)
      displayBuffer[i] = 0xFF;
    else
      displayBuffer[i] = GetCharMask(str[i]);
  }
}

// Установка глобальной яркости (0-100%)
void Segment_SetBrightness(uint8_t percent) {
  if (percent > 100) percent = 100;
  if (percent < 1) percent = 1;
  GLOBAL_BRIGHTNESS = percent;
}

// Основная функция обновления дисплея
// Вызывается каждые 10 микросекунд из таймера
void Segment_Process(void) {
  UpdateOneDigit();
}
