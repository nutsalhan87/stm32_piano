/*
 * tracks.h
 *
 *  Created on: Dec 13, 2024
 *      Author: nutsa
 */

#ifndef INC_USER_TRACKS_H_
#define INC_USER_TRACKS_H_

#include "user/sound.h"

#define CUSTOM_TRACK_MAX_LENGTH 1024

struct Track {
    struct Sample *samples;
    size_t length;
    struct ADSR adsr;
    const char *name;
    uint32_t base_note_duration; // ms
};

extern struct Track track_fade;
extern struct Track track_kuznechik;
extern struct Track track_megalovania;
extern struct Track track_white_roses;
extern struct Track track_custom;

#endif /* INC_USER_TRACKS_H_ */
