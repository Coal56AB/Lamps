#ifndef MELODY_H
#define MELODY_H

#include "stm32f1xx_hal.h"
#include "songs.h"
#include "sounds.h"

typedef struct {
    TIM_HandleTypeDef* htim;
    uint32_t channel;
    uint32_t timer_clock_hz;   // частота тактирования таймера в Гц
    Melody_t *melody;
    uint16_t current_index;
    uint32_t note_start_time;
    uint8_t is_playing;
    uint8_t volume;
    uint16_t bpm;
} MelodyHandle;
extern MelodyHandle melody;

void Melody_Init(MelodyHandle* mh, TIM_HandleTypeDef* htim, uint32_t channel, uint32_t timer_clock_hz);
void Melody_SetBPM(MelodyHandle* mh, uint16_t bpm);
void Melody_Play(MelodyHandle* mh, Melody_t* melody, uint16_t bpm);
void Melody_Stop(MelodyHandle* mh);
void Melody_Update(MelodyHandle* mh);
uint8_t Melody_IsPlaying(MelodyHandle* mh);
void Melody_SetVolume(MelodyHandle* mh, uint8_t volume);
uint8_t Melody_GetVolume(MelodyHandle* mh);
uint8_t Melody_GetSongCount(void);
const char* Melody_GetSongName(uint8_t song);
uint8_t Melody_PlaySong(MelodyHandle* mh, uint8_t song);

#endif
