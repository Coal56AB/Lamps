#include "melody.h"

MelodyHandle melody;
#define FIXED_ARR 1000  // фиксированный период таймера
#define FIXED_VOLUME 2  // фиксированный период таймера

// Конвертирует длительность ноты в миллисекунды
static uint32_t _duration_to_ms(MelodyHandle* mh, float duration) {
    float quarter_sec = 60.0f / mh->bpm;           // четверть в секундах
    float whole_sec = quarter_sec * 4.0f;          // целая нота в секундах
    float note_sec = whole_sec * duration;         // нота в секундах
    return (uint32_t)(note_sec * 1000.0f);         // в миллисекунды
}

// Устанавливает частоту на выходе PWM
static void _set_freq(MelodyHandle* mh, uint32_t freq) {
    if (freq == 0) {
        // Выключаем звук - просто CCR = 0
        __HAL_TIM_SET_COMPARE(mh->htim, mh->channel, 0);
        return;
    }
    
    // Считаем предделитель: PSC = F_timer / freq / (ARR+1)
    // ARR+1 = 11, так как ARR = 10
    uint32_t psc = mh->timer_clock_hz / freq / (FIXED_ARR + 1);
    
    // Ограничиваем диапазоном 1..65535
    if (psc < 1) psc = 1;
    if (psc > 65535) psc = 65535;
    
    // Устанавливаем prescaler (вычитаем 1, т.к. счётчик от 0 до psc-1)
    __HAL_TIM_SET_PRESCALER(mh->htim, psc - 1);
    // Фиксированный период
    __HAL_TIM_SET_AUTORELOAD(mh->htim, FIXED_ARR);
    // Скважность 50% для чистого тона
    __HAL_TIM_SET_COMPARE(mh->htim, mh->channel, FIXED_VOLUME);
}

void Melody_Init(MelodyHandle* mh, TIM_HandleTypeDef* htim, uint32_t channel, uint32_t timer_clock_hz) {
    mh->htim = htim;
    mh->channel = channel;
    mh->timer_clock_hz = timer_clock_hz;
    mh->melody = NULL;
    mh->current_index = 0;
    mh->note_start_time = 0;
    mh->is_playing = 0;
    
    // Настраиваем таймер на фиксированный период
    __HAL_TIM_SET_AUTORELOAD(mh->htim, FIXED_ARR);
    __HAL_TIM_SET_COMPARE(mh->htim, mh->channel, 0);
    
    // Запускаем PWM один раз
    HAL_TIM_PWM_Start(mh->htim, mh->channel);
}

void Melody_SetBPM(MelodyHandle* mh, uint16_t bpm) {
    mh->bpm = bpm;
}

void Melody_Play(MelodyHandle* mh, Melody_t *melody, uint16_t bpm) {
    if (mh->is_playing) {
        _set_freq(mh, 0);
    }
    
    mh->melody = melody;
    mh->current_index = 0;
    mh->is_playing = 1;
    mh->note_start_time = HAL_GetTick();
    mh->bpm = bpm;
    
    if (melody->length > 0 && melody->sequence[0].freq != 0) {
        _set_freq(mh, melody->sequence[0].freq);
    }
}

void Melody_Stop(MelodyHandle* mh) {
    mh->is_playing = 0;
    _set_freq(mh, 0);
}

void Melody_Update(MelodyHandle* mh) {
    if (!mh->is_playing) return;
    
    uint32_t now = HAL_GetTick();
    uint32_t dur_ms = _duration_to_ms(mh, mh->melody->sequence[mh->current_index].duration);
    
    if (now - mh->note_start_time >= dur_ms) {
        mh->current_index++;
        
        if (mh->current_index >= mh->melody->length) {
            Melody_Stop(mh);
            return;
        }
        
        mh->note_start_time = now;
        
        if (mh->melody->sequence[mh->current_index].freq == NOTE_REST) {
            __HAL_TIM_SET_COMPARE(mh->htim, mh->channel, 0);
        } else {
            _set_freq(mh, mh->melody->sequence[mh->current_index].freq);
        }
    }
}

uint8_t Melody_IsPlaying(MelodyHandle* mh) {
    return mh->is_playing;
}
