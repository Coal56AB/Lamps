#pragma once

#include "notes.h"

#define song_length(song_arr) sizeof(song_arr)/sizeof(song_arr[0])
#define SFX_BPM 480  // 480 BPM для звуковых эффектов

// ==================== ЗВУКОВЫЕ ЭФФЕКТЫ ====================

// Короткий писк
static const MelodyStep_t SFX_Beep_Notes[] = {
    {{DO4, NOTE_SIXTEENTH}, {NOTE_REST, NOTE_SIXTEENTH}}
};
static Melody_t SFX_Beep = {song_length(SFX_Beep_Notes), SFX_Beep_Notes};

// Двойной писк
static const MelodyStep_t SFX_DoubleBeep_Notes[] = {
    {{DO4, NOTE_SIXTEENTH}, {NOTE_REST, NOTE_SIXTEENTH}},
    {{NOTE_REST, NOTE_SIXTEENTH}, {NOTE_REST, NOTE_SIXTEENTH}},
    {{DO4, NOTE_SIXTEENTH}, {NOTE_REST, NOTE_SIXTEENTH}}
};
static Melody_t SFX_DoubleBeep = {song_length(SFX_DoubleBeep_Notes), SFX_DoubleBeep_Notes};

// Ошибка
static const MelodyStep_t SFX_Error_Notes[] = {
    {{DO4, NOTE_EIGHTH}, {NOTE_REST, NOTE_EIGHTH}},
    {{NOTE_REST, NOTE_EIGHTH}, {NOTE_REST, NOTE_EIGHTH}},
    {{DO4, NOTE_EIGHTH}, {NOTE_REST, NOTE_EIGHTH}}
};
static Melody_t SFX_Error = {song_length(SFX_Error_Notes), SFX_Error_Notes};

// Успех
static const MelodyStep_t SFX_Success_Notes[] = {
    {{DO4, NOTE_SIXTEENTH}, {NOTE_REST, NOTE_SIXTEENTH}},
    {{MI4, NOTE_SIXTEENTH}, {NOTE_REST, NOTE_SIXTEENTH}},
    {{SOL4, NOTE_QUARTER}, {NOTE_REST, NOTE_QUARTER}}
};
static Melody_t SFX_Success = {song_length(SFX_Success_Notes), SFX_Success_Notes};

// Нажатие кнопки
static const MelodyStep_t SFX_Click_Notes[] = {
    {{DO4, NOTE_THIRTYSECOND}, {NOTE_REST, NOTE_THIRTYSECOND}}
};
static Melody_t SFX_Click = {song_length(SFX_Click_Notes), SFX_Click_Notes};

// Тревога
static const MelodyStep_t SFX_Alarm_Notes[] = {
    {{LA4, NOTE_EIGHTH}, {NOTE_REST, NOTE_EIGHTH}},
    {{NOTE_REST, NOTE_EIGHTH}, {NOTE_REST, NOTE_EIGHTH}},
    {{LA4, NOTE_EIGHTH}, {NOTE_REST, NOTE_EIGHTH}},
    {{NOTE_REST, NOTE_EIGHTH}, {NOTE_REST, NOTE_EIGHTH}},
    {{LA4, NOTE_EIGHTH}, {NOTE_REST, NOTE_EIGHTH}}
};
static Melody_t SFX_Alarm = {song_length(SFX_Alarm_Notes), SFX_Alarm_Notes};

// Отдельная короткая фанфара для победы в игре
static const MelodyStep_t SFX_Victory_Notes[] = {
    {{DO4, NOTE_EIGHTH}, {NOTE_REST, NOTE_EIGHTH}},
    {{MI4, NOTE_EIGHTH}, {NOTE_REST, NOTE_EIGHTH}},
    {{SOL4, NOTE_EIGHTH}, {NOTE_REST, NOTE_EIGHTH}},
    {{DO4, NOTE_QUARTER}, {NOTE_REST, NOTE_QUARTER}},
    {{NOTE_REST, NOTE_SIXTEENTH}, {NOTE_REST, NOTE_SIXTEENTH}},
    {{SOL4, NOTE_SIXTEENTH}, {NOTE_REST, NOTE_SIXTEENTH}},
    {{DO4, NOTE_QUARTER}, {NOTE_REST, NOTE_QUARTER}}
};
static Melody_t SFX_Victory = {song_length(SFX_Victory_Notes), SFX_Victory_Notes};
