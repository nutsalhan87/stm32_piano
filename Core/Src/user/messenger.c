/*
 * messenger.c
 *
 *  Created on: Nov 19, 2024
 *      Author: nutsa
 */


#include "user/async.h"
#include "user/messenger.h"
#include "usart.h"
#include "stm32f4xx_hal.h"
#include "string.h"

#define BUFFER_CAPACITY 512

static char ring_buffer[BUFFER_CAPACITY][MAX_LINE_LENGTH];
static size_t buffer_cursor = 0; // where to write

static bool messenger_read_mutex = false;

bool messenger_read_signal = false;

async_impl(messenger_read) async_body({
    start_fn();
	await(1, ml, mutex_lock, &messenger_read_mutex);
	while (HAL_UART_Receive_IT(&huart6, &fn_state->ans, 1) == HAL_BUSY) {
		yield(2);
	}
	await(3, s, await_signal, &messenger_read_signal);
	await(4, mu, mutex_unlock, &messenger_read_mutex);
	end_fn(5);
})

async_impl(messenger_write, const char *msg, size_t length) async_body({
	start_fn();
	if (length > MAX_LINE_LENGTH) {
		length = MAX_LINE_LENGTH;
	}
	fn_state->current_cursor = buffer_cursor; 
	buffer_cursor = (buffer_cursor + 1) % BUFFER_CAPACITY;
	strncpy(ring_buffer[fn_state->current_cursor], msg, length);
	while (HAL_UART_Transmit_IT(&huart6, ring_buffer[fn_state->current_cursor], length) == HAL_BUSY) {
		yield(1);
	}
	end_fn(2);
})
