/*
 * util.h
 *
 *  Created on: 22 нояб. 2024 г.
 *      Author: nutsa
 */

#ifndef INC_USER_UTIL_H_
#define INC_USER_UTIL_H_

#include "user/async.h"
#include "stdint.h"
#include "stdbool.h"

async_def(async_delay, { uint32_t start_tick; }, uint32_t delay);

async_def(await_signal, no_fields, bool *signal);

async_def(mutex_lock, no_fields, bool *mutex);

async_def(mutex_unlock, no_fields, bool *mutex);

#endif /* INC_USER_UTIL_H_ */
