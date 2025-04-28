/*
 * util.c
 *
 *  Created on: 22 нояб. 2024 г.
 *      Author: nutsa
 */


#include "user/util.h"
#include "gpio.h"

async_impl(async_delay, uint32_t delay) async_body({
	start_fn();
	fn_state->start_tick = HAL_GetTick();
	fn_state->step = 1; case 1:;
	uint32_t current_tick = HAL_GetTick();
	if ((current_tick - fn_state->start_tick) < delay) {
		return ASYNC_IN_PROGRESS;
	} else {
		end_fn(2);
	}
})

async_impl(await_signal, bool *signal) async_body({
	start_fn();
	while (*signal == false) {
		yield(1);
	}
	*signal = false;
	end_fn(2);
})

async_impl(mutex_lock, bool *mutex) async_body({
	start_fn();
	while (*mutex == true) {
		yield(1);
	}
	*mutex = true;
	end_fn(2);
})

async_impl(mutex_unlock, bool *mutex) async_body({
	start_fn();
	*mutex = false;
	end_fn(1);
})