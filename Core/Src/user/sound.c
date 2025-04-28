/*
 * sound.c
 *
 *  Created on: Dec 11, 2024
 *      Author: nutsa
 */


#include "user/sound.h"
#include "tim.h"
#include "math.h"
#include "stdint.h"

struct PitchCharacteristics {
    uint16_t prescaler;
    uint16_t counter_period;
};

static struct PitchCharacteristics pitch_characteristics[9][12] = {
	    {
	        { 330, 9622 },
	        { 4596, 653 },
	        { 3220, 880 },
	        { 84, 31510 },
	        { 293, 8598 },
	        { 138, 17166 },
	        { 161, 13902 },
	        { 374, 5668 },
	        { 2452, 817 },
	        { 2830, 668 },
	        { 2418, 738 },
	        { 2652, 635 },
	    },
	    {
	        { 34, 45502 },
	        { 30, 48490 },
	        { 2101, 674 },
	        { 2320, 576 },
	        { 146, 8598 },
	        { 1804, 660 },
	        { 80, 13902 },
	        { 213, 4966 },
	        { 408, 2452 },
	        { 1684, 561 },
	        { 59, 14896 },
	        { 1136, 741 },
	    },
	    {
	        { 1493, 532 },
	        { 1313, 571 },
	        { 861, 822 },
	        { 422, 1582 },
	        { 985, 640 },
	        { 13, 42610 },
	        { 847, 663 },
	        { 106, 4966 },
	        { 39, 12540 },
	        { 336, 1404 },
	        { 29, 14896 },
	        { 378, 1112 },
	    },
	    {
	        { 633, 627 },
	        { 656, 571 },
	        { 430, 822 },
	        { 446, 748 },
	        { 492, 640 },
	        { 6, 42610 },
	        { 423, 663 },
	        { 517, 512 },
	        { 19, 12540 },
	        { 421, 560 },
	        { 14, 14896 },
	        { 21, 9586 },
	    },
	    {
	        { 316, 627 },
	        { 40, 4582 },
	        { 201, 877 },
	        { 162, 1026 },
	        { 92, 1698 },
	        { 21, 6778 },
	        { 211, 663 },
	        { 258, 512 },
	        { 9, 12540 },
	        { 210, 560 },
	        { 15, 6982 },
	        { 10, 9586 },
	    },
	    {
	        { 156, 633 },
	        { 142, 656 },
	        { 100, 877 },
	        { 154, 539 },
	        { 153, 512 },
	        { 10, 6778 },
	        { 105, 663 },
	        { 117, 562 },
	        { 4, 12540 },
	        { 100, 585 },
	        { 7, 6982 },
	        { 103, 506 },
	    },
	    {
	        { 0, 49768 },
	        { 63, 733 },
	        { 0, 44338 },
	        { 0, 41850 },
	        { 1, 19750 },
	        { 4, 7456 },
	        { 52, 663 },
	        { 58, 562 },
	        { 7, 3918 },
	        { 0, 29592 },
	        { 3, 6982 },
	        { 51, 506 },
	    },
	    {
	        { 3, 6220 },
	        { 31, 733 },
	        { 29, 738 },
	        { 30, 674 },
	        { 0, 19750 },
	        { 25, 716 },
	        { 3, 4398 },
	        { 31, 518 },
	        { 3, 3918 },
	        { 26, 547 },
	        { 1, 6982 },
	        { 25, 506 },
	    },
	    {
	        { 1, 6220 },
	        { 15, 733 },
	        { 14, 738 },
	        { 0, 10462 },
	        { 4, 1974 },
	        { 12, 716 },
	        { 1, 4398 },
	        { 15, 518 },
	        { 1, 3918 },
	        { 8, 821 },
	        { 0, 6982 },
	        { 12, 506 },
	    }
};

static struct PitchCharacteristics characteristics_from_pitch(struct Pitch pitch) {
    if (pitch.octave > 8) {
        pitch.octave = 8;
    }
    return pitch_characteristics[pitch.octave][pitch.note];
}

static float volume_clamp(float volume) {
    if (volume > 1.) {
        return 1.;
    } else if (volume < 0.) {
        return 0.;
    } else {
        return volume;
    }
}

static float volume_curved(float volume, uint16_t counter_period) {
    return 0.4 * powf(volume_clamp(volume), 2.7777) * counter_period;
}

void sound_volume_off() {
    htim1.Instance->CCR1 = 0;
}

async_impl(sound_linear_fade, struct Pitch pitch, float volume_start, float volume_end, uint16_t ms) async_body({
    start_fn();
    struct PitchCharacteristics c = characteristics_from_pitch(pitch);
    htim1.Instance->PSC = c.prescaler;
    htim1.Instance->ARR = c.counter_period;
    htim1.Instance->CCR1 = volume_curved(volume_start, c.counter_period);
    fn_state->start_tick = HAL_GetTick();
    yield(1);
    c = characteristics_from_pitch(pitch);
    uint32_t current_tick = HAL_GetTick();
    float elapsed = 1.0 * (current_tick - fn_state->start_tick) / ms;
    htim1.Instance->CCR1 = volume_curved(volume_start + (volume_end - volume_start) * elapsed, c.counter_period);
    if (elapsed >= 1.) {
        end_fn(3);
    } else {
        fn_state->step = 1;
        return ASYNC_IN_PROGRESS;
    }
})

async_impl(sound_play_pitch, struct ADSR adsr, struct Pitch pitch, uint32_t ms) async_body({
    start_fn();
    if (adsr.attack + adsr.decay + adsr.release > ms) {
        fn_state->sustain_time = 0;
    } else {
        fn_state->sustain_time = ms - (adsr.attack + adsr.decay + adsr.release);
    }
    await(1, s, sound_linear_fade, pitch, 0., 1., adsr.attack);
    await(2, s, sound_linear_fade, pitch, 1., adsr.sustain, adsr.decay);
    await(3, s, sound_linear_fade, pitch, adsr.sustain, adsr.sustain, fn_state->sustain_time);
    await(4, s, sound_linear_fade, pitch, adsr.sustain, 0., adsr.release);
    end_fn(5);
})

async_impl(sound_play_samples, struct ADSR adsr, struct Sample *samples, size_t length) async_body({
    start_fn();
    fn_state->idx = 0;
    while (fn_state->idx < length) {
        if (samples[fn_state->idx].is_zero) {
            htim1.Instance->CCR1 = 0;
            await(1, a, async_delay, 200 * samples[fn_state->idx].duration_modifier);
        } else {
            await(2, s, sound_play_pitch, adsr, samples[fn_state->idx].pitch, 200 * samples[fn_state->idx].duration_modifier);
        }
        fn_state->idx++;
    }
    end_fn(3);
})
