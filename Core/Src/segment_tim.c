#include "segment.h"
#include "main.h"

// ==================== ТАЙМЕР ДЛЯ ПРЕРЫВАНИЙ ====================
#define SEGMENT_PROCESS_TIMER      htim1

// ==================== КОНФИГУРАЦИЯ ДИСПЛЕЯ ====================
#define DIGITS_COUNT          6
#define MULTIPLEX_FREQ_HZ     10000
#define PWM_FREQUENCY_HZ      10000
#define PWM_RESOLUTION        100
#define TIMER_BUS_FREQ_MHZ    72

#define SWAP_BIT5_BIT6(x) (((x) & 0x9F) | (((x) & 0x20) << 1) | (((x) & 0x40) >> 1))

// ==================== ИНИЦИАЛИЗАЦИЯ СЕГМЕНТОВ ====================
SegCtrl_t segments[7] = {
  SEG_A_CONFIG,
  SEG_B_CONFIG,
  SEG_C_CONFIG,
  SEG_D_CONFIG,
  SEG_E_CONFIG,
  SEG_F_CONFIG,
  SEG_G_CONFIG
};

// ==================== ТАБЛИЦА СЕГМЕНТОВ ====================
// 1 - сегмент включен, 0 - выключен
static const uint8_t segmentTable[10] = {
  0x3F,  // 0: 0011 1111 - A,B,C,D,E,F
  0x06,  // 1: 0000 0110 - B,C
  0x5B,  // 2: 0101 1011 - A,B,D,E,G
  0x4F,  // 3: 0100 1111 - A,B,C,D,G
  0x66,  // 4: 0110 0110 - B,C,F,G
  0x6D,  // 5: 0110 1101 - A,C,D,F,G
  0x7D,  // 6: 0111 1101 - A,C,D,E,F,G
  0x07,  // 7: 0000 0111 - A,B,C
  0x7F,  // 8: 0111 1111 - все сегменты
  0x6F   // 9: 0110 1111 - A,B,C,D,F,G
};

static const uint8_t activeSegmentsCount[10] = {
  6, 2, 5, 5, 4, 5, 6, 3, 7, 6
};

// ==================== ПЕРЕМЕННЫЕ ====================
static uint8_t displayBuffer[DIGITS_COUNT];
static uint8_t currentPos = 0;
static TimeStruct currentTime;
static uint8_t globalBrightness = 100;
static uint8_t digitCompensation[10];
static uint32_t switchIntervalTicks;
static uint32_t tickCounter = 0;

// ==================== ФУНКЦИИ УПРАВЛЕНИЯ РАЗРЯДАМИ ====================

static void DisableAllDigits(void) {
  DIGIT_HOUR_H_GPIO_Port->BSRR = DIGIT_HOUR_H_Pin << 16;
  DIGIT_HOUR_L_GPIO_Port->BSRR = DIGIT_HOUR_L_Pin << 16;
  DIGIT_MIN_H_GPIO_Port->BSRR = DIGIT_MIN_H_Pin << 16;
  DIGIT_MIN_L_GPIO_Port->BSRR = DIGIT_MIN_L_Pin << 16;
  DIGIT_SEC_H_GPIO_Port->BSRR = DIGIT_SEC_H_Pin << 16;
  DIGIT_SEC_L_GPIO_Port->BSRR = DIGIT_SEC_L_Pin << 16;
}

static void EnableDigit(uint8_t pos) {
  switch(pos) {
    case 0: DIGIT_HOUR_H_GPIO_Port->BSRR = DIGIT_HOUR_H_Pin; break;
    case 1: DIGIT_HOUR_L_GPIO_Port->BSRR = DIGIT_HOUR_L_Pin; break;
    case 2: DIGIT_MIN_H_GPIO_Port->BSRR = DIGIT_MIN_H_Pin; break;
    case 3: DIGIT_MIN_L_GPIO_Port->BSRR = DIGIT_MIN_L_Pin; break;
    case 4: DIGIT_SEC_H_GPIO_Port->BSRR = DIGIT_SEC_H_Pin; break;
    case 5: DIGIT_SEC_L_GPIO_Port->BSRR = DIGIT_SEC_L_Pin; break;
  }
}

// ==================== ИНИЦИАЛИЗАЦИЯ CCMR ДЛЯ КАЖДОГО СЕГМЕНТА ====================

static void InitChannel(SegCtrl_t *seg) {
  // Определяем указатель на CCMR регистр и сдвиг в зависимости от канала
  if (seg->channel == TIM_CHANNEL_1) {
    seg->ccmr_ptr = (uint32_t*)&seg->htim->Instance->CCMR1;
    seg->ccmr_shift = 0;
    if(!seg->isComplementary)
      seg->htim->Instance->CCER &= ~TIM_CCER_CC1P; // Сброс бита CC1P
    else
      seg->htim->Instance->CCER |= TIM_CCER_CC1P; // Установка бита CC1P
    
  } else if (seg->channel == TIM_CHANNEL_2) {
    seg->ccmr_ptr = (uint32_t*)&seg->htim->Instance->CCMR1;
    seg->ccmr_shift = 8;
    if(!seg->isComplementary)
      seg->htim->Instance->CCER &= ~TIM_CCER_CC2P; // Сброс бита CC2P
    else
      seg->htim->Instance->CCER |= TIM_CCER_CC2P; // Установка бита CC2P
    
  } else if (seg->channel == TIM_CHANNEL_3) {
    seg->ccmr_ptr = (uint32_t*)&seg->htim->Instance->CCMR2;
    seg->ccmr_shift = 0;
    if(!seg->isComplementary)
      seg->htim->Instance->CCER &= ~TIM_CCER_CC3P; // Сброс бита CC3P
    else
      seg->htim->Instance->CCER |= TIM_CCER_CC3P; // Установка бита CC3P
    
  } else if (seg->channel == TIM_CHANNEL_4) {
    seg->ccmr_ptr = (uint32_t*)&seg->htim->Instance->CCMR2;
    seg->ccmr_shift = 8;
    if(!seg->isComplementary)
      seg->htim->Instance->CCER &= ~TIM_CCER_CC4P; // Сброс бита CC4P
    else
      seg->htim->Instance->CCER |= TIM_CCER_CC4P; // Установка бита CC4P
    
  }
}

// ==================== ФУНКЦИИ УПРАВЛЕНИЯ ШИМ ====================

static inline void PWM_StartChannel(SegCtrl_t *seg) {
  if (seg->isComplementary) {
    HAL_TIMEx_PWMN_Start(seg->htim, seg->channel);
  } else {
    HAL_TIM_PWM_Start(seg->htim, seg->channel);
  }
}

static inline void PWM_SetMode(SegCtrl_t *seg, uint32_t mode) {
  uint32_t mask = 0x7 << (seg->ccmr_shift+4);
  *seg->ccmr_ptr &= ~mask;
  *seg->ccmr_ptr |= (mode << seg->ccmr_shift);
}

static inline void PWM_SetDuty(SegCtrl_t *seg, uint32_t duty) {
  uint32_t final_duty = duty;
  
  if (duty > PWM_RESOLUTION) duty = PWM_RESOLUTION;
  
  if (final_duty == 0) {
    PWM_SetMode(seg, TIM_OCMODE_FORCED_INACTIVE);
  } else {
    PWM_SetMode(seg, TIM_OCMODE_PWM1);
    __HAL_TIM_SET_COMPARE(seg->htim, seg->channel, final_duty);
  }
}

// ==================== АВТОНАСТРОЙКА ТАЙМЕРА ====================

static void TimerAutoConfig(TIM_HandleTypeDef *htim) {
  uint32_t timer_clock_hz = TIMER_BUS_FREQ_MHZ * 1000000;
  
  uint32_t arr = PWM_RESOLUTION - 1;
  uint32_t prescaler_plus1 = timer_clock_hz / (PWM_FREQUENCY_HZ * PWM_RESOLUTION);
  
  if (prescaler_plus1 < 1) prescaler_plus1 = 1;
  if (prescaler_plus1 > 65535) prescaler_plus1 = 65535;
  
  uint32_t prescaler = prescaler_plus1 - 1;
  
  __HAL_TIM_SET_PRESCALER(htim, prescaler);
  __HAL_TIM_SET_AUTORELOAD(htim, arr);
}

// ==================== ФУНКЦИИ УПРАВЛЕНИЯ СЕГМЕНТАМИ ====================

static void SetSegment(uint8_t segIndex, uint8_t state) {
  SegCtrl_t *seg = &segments[segIndex];
  seg->isActive = state;
  if (state) {
    PWM_SetDuty(seg, seg->Duty);
  } else {
    PWM_SetDuty(seg, 0);
  }
}

static void SetSegmentBrightness(uint8_t segIndex, uint8_t percent) {
  SegCtrl_t *seg = &segments[segIndex];
  if (percent > 100) percent = 100;
  seg->Duty = (percent * PWM_RESOLUTION) / 100;
}

static void DisplayDigit(uint8_t digit, uint8_t pos) {
  if (digit > 9) digit = 0;
  uint8_t segmentMask = segmentTable[digit];
  
  if (pos == 5) {
    segmentMask = SWAP_BIT5_BIT6(segmentMask);
  }
  
  // Применяем компенсацию для текущей цифры
  uint8_t comp = digitCompensation[digit];
  uint8_t finalBrightness = (globalBrightness * comp) / 100;
  
  for (int i = 0; i < 7; i++) {
    SetSegmentBrightness(i, finalBrightness);
    if ((segmentMask >> i) & 1) {
      SetSegment(i, 1);
    } else {
      SetSegment(i, 0);
    }
  }
}

// ==================== ФУНКЦИИ ОБНОВЛЕНИЯ БУФЕРА ====================

static void UpdateDisplayBuffer(void) {
  uint8_t hours = currentTime.hours;
  uint8_t minutes = currentTime.minutes;
  uint8_t seconds = currentTime.seconds;
  
  static const uint8_t div10[100] = {
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,
    4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9
  };
  
  static const uint8_t mod10[100] = {
    0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
    0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9
  };
  
  displayBuffer[0] = div10[hours];
  displayBuffer[1] = mod10[hours];
  displayBuffer[2] = div10[minutes];
  displayBuffer[3] = mod10[minutes];
  displayBuffer[4] = div10[seconds];
  displayBuffer[5] = mod10[seconds];
}

static void NextDigit(void) {
  DisableAllDigits();
  
  currentPos++;
  if (currentPos >= DIGITS_COUNT) {
    currentPos = 0;
  }
  
  DisplayDigit(displayBuffer[currentPos], currentPos);
  EnableDigit(currentPos);
}

// ==================== ПУБЛИЧНЫЕ ФУНКЦИИ ====================

void Segment_Init(void) {
  TIM_HandleTypeDef* configuredTimers[10] = {0};
  int timerCount = 0;
  
  // Инициализируем CCMR для каждого сегмента
  for (int i = 0; i < 7; i++) {
    InitChannel(&segments[i]);
  }
  
  // Настраиваем и запускаем все уникальные таймеры для ШИМ
  for (int i = 0; i < 7; i++) {
    TIM_HandleTypeDef *htim = segments[i].htim;
    
    int alreadyConfigured = 0;
    for (int j = 0; j < timerCount; j++) {
      if (configuredTimers[j] == htim) {
        alreadyConfigured = 1;
        break;
      }
    }
    
    if (!alreadyConfigured) {
      TimerAutoConfig(htim);
      configuredTimers[timerCount++] = htim;
    }
    
    PWM_StartChannel(&segments[i]);
  }
  
  // Инициализация сегментов
  for (int i = 0; i < 7; i++) {
    segments[i].Duty = 0;
    segments[i].isActive = 0;
    PWM_SetDuty(&segments[i], 0);
  }
  
  // Рассчитываем компенсацию яркости для каждой цифры
  // Чем меньше сегментов у цифры, тем ярче должен гореть каждый сегмент
  for (int i = 0; i < 10; i++) {
    // Максимальное количество сегментов = 7 (цифра 8)
    // Коэффициент = (7 / количество_сегментов_у_цифры) * 100
    digitCompensation[i] = (activeSegmentsCount[i] * 100) / 7;
    if (digitCompensation[i] > 200) digitCompensation[i] = 200; // Ограничиваем
  }
  
  currentTime.hours = 0;
  currentTime.minutes = 0;
  currentTime.seconds = 0;
  UpdateDisplayBuffer();
  
  switchIntervalTicks = PWM_FREQUENCY_HZ / MULTIPLEX_FREQ_HZ;
  if (switchIntervalTicks < 1) switchIntervalTicks = 1;
  tickCounter = 0;
  
  DisplayDigit(displayBuffer[0], 0);
  EnableDigit(0);
  
  // Запускаем таймер прерываний
  HAL_TIM_Base_Start_IT(&SEGMENT_PROCESS_TIMER);
}

void Segment_SetBrightness(uint8_t percent) {
  if (percent > 100) percent = 100;
  globalBrightness = percent;
  DisplayDigit(displayBuffer[currentPos], currentPos);
}

void Segment_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
  if (hours > 23) hours = 23;
  if (minutes > 59) minutes = 59;
  if (seconds > 59) seconds = 59;
  
  currentTime.hours = hours;
  currentTime.minutes = minutes;
  currentTime.seconds = seconds;
  UpdateDisplayBuffer();
}

void Segment_Process(void) {
  tickCounter++;
  if (tickCounter >= switchIntervalTicks) {
    tickCounter = 0;
    NextDigit();
  }
}