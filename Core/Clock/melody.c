#include "melody.h"

MelodyHandle melody;

#define DEFAULT_VOLUME 5
#define MAX_VOLUME 10
#define VOLUME_COMPARE_STEP 5
#define FIXED_ARR 1000

typedef struct {
    const char *name;
    Melody_t *melody;
    uint16_t bpm;
} SongEntry;

static const SongEntry songCatalog[] = {
    {"OD",  &Polyphia_OD,         136/2},
    {"GOD", &Polyphia_PlayingGod, 134}
};

static uint32_t _duration_to_ms(MelodyHandle* mh, float duration) {
    float quarter_sec = 60.0f / mh->bpm;
    float whole_sec = quarter_sec * 4.0f;
    uint32_t duration_ms = (uint32_t)(whole_sec * duration * 1000.0f);
    return duration_ms == 0 ? 1 : duration_ms;
}

static const Note_t* _current_note(MelodyHandle* mh, uint8_t voice) {
    return &mh->melody->sequence[mh->voices[voice].current_index][voice];
}

static void _set_pwm_freq(MelodyHandle* mh, uint8_t voice, uint32_t freq) {
    MelodyOutput *output = &mh->outputs[voice];
    uint32_t psc;

    if (output->htim == NULL) return;

    if (freq == NOTE_REST || mh->volume == 0) {
        __HAL_TIM_SET_COMPARE(output->htim, output->channel, 0);
        return;
    }

    psc = output->timer_clock_hz / freq / (FIXED_ARR + 1U);
    if (psc < 1U) psc = 1U;
    if (psc > 65535U) psc = 65535U;

    __HAL_TIM_SET_PRESCALER(output->htim, psc - 1U);
    __HAL_TIM_SET_AUTORELOAD(output->htim, FIXED_ARR);
    __HAL_TIM_SET_COUNTER(output->htim, 0);
    output->htim->Instance->EGR = TIM_EGR_UG;
    __HAL_TIM_SET_COMPARE(output->htim, output->channel,
                          (uint32_t)mh->volume * VOLUME_COMPARE_STEP);
}

static void _set_voice_freq(MelodyHandle* mh, uint8_t voice, uint16_t freq) {
    _set_pwm_freq(mh, voice, freq);
}

void Melody_Init(MelodyHandle* mh,
                 TIM_HandleTypeDef* htim_a, uint32_t channel_a,
                 uint32_t timer_clock_a_hz,
                 TIM_HandleTypeDef* htim_b, uint32_t channel_b,
                 uint32_t timer_clock_b_hz) {
    uint8_t voice;

    mh->outputs[0].htim = htim_a;
    mh->outputs[0].channel = channel_a;
    mh->outputs[0].timer_clock_hz = timer_clock_a_hz;
    mh->outputs[1].htim = htim_b;
    mh->outputs[1].channel = channel_b;
    mh->outputs[1].timer_clock_hz = timer_clock_b_hz;
    mh->melody = NULL;
    mh->is_playing = 0;
    mh->volume = DEFAULT_VOLUME;
    mh->bpm = 120;

    for (voice = 0; voice < 2U; voice++) {
        mh->voices[voice].current_index = 0;
        mh->voices[voice].note_start_time = 0;
        mh->voices[voice].is_playing = 0;
        if (mh->outputs[voice].htim == NULL) continue;
        __HAL_TIM_SET_AUTORELOAD(mh->outputs[voice].htim, FIXED_ARR);
        __HAL_TIM_SET_COMPARE(mh->outputs[voice].htim,
                              mh->outputs[voice].channel, 0);
        HAL_TIM_PWM_Start(mh->outputs[voice].htim,
                          mh->outputs[voice].channel);
    }
}

void Melody_SetBPM(MelodyHandle* mh, uint16_t bpm) {
    if (bpm > 0U) mh->bpm = bpm;
}

void Melody_Play(MelodyHandle* mh, Melody_t *new_melody, uint16_t bpm) {
    uint8_t voice;
    uint32_t now = HAL_GetTick();

    Melody_Stop(mh);
    mh->melody = new_melody;
    mh->bpm = bpm > 0U ? bpm : 120;

    if (new_melody == NULL || new_melody->length == 0U) return;

    mh->is_playing = 1;
    for (voice = 0; voice < 2U; voice++) {
        mh->voices[voice].current_index = 0;
        mh->voices[voice].note_start_time = now;
        mh->voices[voice].is_playing = 1;
        _set_voice_freq(mh, voice, new_melody->sequence[0][voice].freq);
    }
}

void Melody_Stop(MelodyHandle* mh) {
    mh->is_playing = 0;
    mh->voices[0].is_playing = 0;
    mh->voices[1].is_playing = 0;
    _set_voice_freq(mh, 0, NOTE_REST);
    _set_voice_freq(mh, 1, NOTE_REST);
}

void Melody_Update(MelodyHandle* mh) {
    uint8_t voice;
    uint8_t any_playing = 0;
    uint32_t now;

    if (!mh->is_playing || mh->melody == NULL) return;
    now = HAL_GetTick();

    for (voice = 0; voice < 2U; voice++) {
        MelodyVoice *state = &mh->voices[voice];
        uint32_t duration_ms;

        if (!state->is_playing) continue;
        duration_ms = _duration_to_ms(mh, _current_note(mh, voice)->duration);

        if (now - state->note_start_time >= duration_ms) {
            state->current_index++;
            state->note_start_time = now;

            if (state->current_index >= mh->melody->length) {
                state->is_playing = 0;
                _set_voice_freq(mh, voice, NOTE_REST);
            } else {
                _set_voice_freq(mh, voice, _current_note(mh, voice)->freq);
            }
        }

        if (state->is_playing) any_playing = 1;
    }

    mh->is_playing = any_playing;
}

uint8_t Melody_IsPlaying(MelodyHandle* mh) {
    return mh->is_playing;
}

void Melody_SetVolume(MelodyHandle* mh, uint8_t volume) {
    uint8_t voice;

    if (volume > MAX_VOLUME) volume = MAX_VOLUME;
    mh->volume = volume;

    for (voice = 0; voice < 2U; voice++) {
        if (mh->is_playing && mh->melody != NULL &&
            mh->voices[voice].is_playing) {
            _set_voice_freq(mh, voice, _current_note(mh, voice)->freq);
        } else {
            _set_voice_freq(mh, voice, NOTE_REST);
        }
    }
}

uint8_t Melody_GetVolume(MelodyHandle* mh) {
    return mh->volume;
}

uint8_t Melody_GetSongCount(void) {
    return (uint8_t)(sizeof(songCatalog) / sizeof(songCatalog[0]));
}

const char* Melody_GetSongName(uint8_t song) {
    if (song == 0U || song > Melody_GetSongCount()) return "OFF";
    return songCatalog[song - 1U].name;
}

uint8_t Melody_PlaySong(MelodyHandle* mh, uint8_t song) {
    if (song == 0U || song > Melody_GetSongCount()) return 0;
    Melody_Play(mh, songCatalog[song - 1U].melody,
                songCatalog[song - 1U].bpm);
    return 1;
}
