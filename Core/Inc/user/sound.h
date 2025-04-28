/*
 * sound.h
 *
 *  Created on: Dec 11, 2024
 *      Author: nutsa
 */

#ifndef INC_SOUND_H_
#define INC_SOUND_H_

#include "user/async.h"
#include "user/util.h"
#include "stdint.h"
#include "stdbool.h"

struct ADSR {
    uint16_t attack; // ms
    uint16_t decay; // ms
    float sustain; // [0; 1]
    uint16_t release; // ms
};

enum Note {
    C, Cd,
    D, Dd,
    E,
    F, Fd,
    G, Gd,
    A, Ad,
    B
};

struct Pitch {
    enum Note note;
    uint8_t octave; // max 8. will be clamped if greater
};

struct Sample {
    bool is_zero;
    struct Pitch pitch; // ignored if zero
    float duration_modifier;
};

void sound_volume_off(void);

async_def(sound_linear_fade, { 
    struct AsyncState_async_delay a;
    uint32_t start_tick;  
}, struct Pitch pitch, float volume_start, float volume_end, uint16_t ms);

async_def(sound_play_pitch, { 
    struct AsyncState_async_delay a;
    struct AsyncState_sound_linear_fade s;
    uint32_t sustain_time;
}, struct ADSR adsr, struct Pitch pitch, uint32_t ms);

async_def(sound_play_samples, {
    struct AsyncState_async_delay a;
    struct AsyncState_sound_play_pitch s;
    size_t idx;
}, struct ADSR adsr, struct Sample *samples, size_t length);

#endif /* INC_SOUND_H_ */