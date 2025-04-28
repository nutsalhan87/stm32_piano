/*
 * messenger.h
 *
 *  Created on: Nov 19, 2024
 *      Author: nutsa
 */

#ifndef INC_MESSENGER_H_
#define INC_MESSENGER_H_

#include "user/async.h"
#include "user/util.h"
#include "stdbool.h"

#define MAX_LINE_LENGTH 256

async_def(messenger_read, { 
    char ans;
    struct AsyncState_await_signal s;
    struct AsyncState_mutex_lock ml;
    struct AsyncState_mutex_unlock mu;
});

async_def(messenger_write, { size_t current_cursor; }, const char *msg, size_t length);

extern bool messenger_read_signal;

#endif /* INC_MESSENGER_H_ */
