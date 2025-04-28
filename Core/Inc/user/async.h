/*
 * async.h
 *
 *  Created on: 22 нояб. 2024 г.
 *      Author: nutsa
 */

#ifndef INC_ASYNC_H_
#define INC_ASYNC_H_

#include "limits.h"

typedef int AsyncFnStep;

#define ASYNC_FN_NEW_STEP 0

enum AsyncResult {
    ASYNC_IN_PROGRESS,
    ASYNC_READY,
};

#define async_def(name, fn_fields, ...)\
    struct AsyncState_##name {\
        AsyncFnStep step;\
        enum AsyncResult result;\
        struct fn_fields;\
    };\
    void async_state_##name##_init(struct AsyncState_##name *fn_state);\
    enum AsyncResult name(struct AsyncState_##name *fn_state __VA_OPT__(,) __VA_ARGS__)

#define async_impl(name, ...) \
    void async_state_##name##_init(struct AsyncState_##name *fn_state) {\
        fn_state->step = ASYNC_FN_NEW_STEP;\
        fn_state->result = ASYNC_IN_PROGRESS;\
    }\
    enum AsyncResult name(struct AsyncState_##name *fn_state __VA_OPT__(,) __VA_ARGS__)

#define no_fields { char _; }

#define async_body(body) {\
    switch (fn_state->step) body\
    return fn_state->result; /* unreachable */ \
    }

#define start_fn() case ASYNC_FN_NEW_STEP:

#define yield(next_step)\
    fn_state->step = next_step;\
    return ASYNC_IN_PROGRESS;\
    case next_step:

#define end_fn(end_step)\
    fn_state->step = end_step;\
    fn_state->result = ASYNC_READY;\
    case end_step:\
    return fn_state->result\

#define await(step_here, state_field, fn, ...)\
    async_state_##fn##_init(&fn_state->state_field);\
    do {\
        fn_state->step = step_here;\
        case step_here:;\
        if (fn(&fn_state->state_field __VA_OPT__(,) __VA_ARGS__) == ASYNC_IN_PROGRESS) {\
            return ASYNC_IN_PROGRESS;\
        }\
    } while (0)

// routines *must* yield so each other can make progress 
#define await_two_routines(step_here, r1_state_field, r1, r2_state_field, r2)\
    async_state_##r1##_init(&fn_state->r1_state_field);\
    async_state_##r2##_init(&fn_state->r2_state_field);\
    fn_state->step = step_here;\
    case step_here:;\
    r1(&fn_state->r1_state_field);\
    r2(&fn_state->r2_state_field);\
    if (fn_state->r1_state_field.result == ASYNC_IN_PROGRESS \
        || fn_state->r2_state_field.result == ASYNC_IN_PROGRESS) {\
        return ASYNC_IN_PROGRESS;\
    }\
    do { } while (0)

#define await_three_routines(step_here, r1_state_field, r1, r2_state_field, r2, r3_state_field, r3)\
    async_state_##r1##_init(&fn_state->r1_state_field);\
    async_state_##r2##_init(&fn_state->r2_state_field);\
    async_state_##r3##_init(&fn_state->r3_state_field);\
    fn_state->step = step_here;\
    case step_here:;\
    r1(&fn_state->r1_state_field);\
    r2(&fn_state->r2_state_field);\
    r3(&fn_state->r3_state_field);\
    if (fn_state->r1_state_field.result == ASYNC_IN_PROGRESS \
        || fn_state->r2_state_field.result == ASYNC_IN_PROGRESS \
        || fn_state->r3_state_field.result == ASYNC_IN_PROGRESS) {\
        return ASYNC_IN_PROGRESS;\
    }\
    do { } while (0)

// routines must save consistent state on each step because next step may never be progressed
#define await_any_of_two_routines(step_here, r1_state_field, r1, r2_state_field, r2)\
    async_state_##r1##_init(&fn_state->r1_state_field);\
    async_state_##r2##_init(&fn_state->r2_state_field);\
    fn_state->step = step_here;\
    case step_here:;\
    r1(&fn_state->r1_state_field);\
    r2(&fn_state->r2_state_field);\
    if (fn_state->r1_state_field.result == ASYNC_IN_PROGRESS \
        && fn_state->r2_state_field.result == ASYNC_IN_PROGRESS) {\
        return ASYNC_IN_PROGRESS;\
    }\
    do { } while (0)

#define await_until(step_here, stop_condition, state_field, fn, ...)\
    async_state_##fn##_init(&fn_state->state_field);\
    do {\
        fn_state->step = step_here;\
        case step_here:;\
        if (!(stop_condition) && fn(&fn_state->state_field __VA_OPT__(,) __VA_ARGS__) == ASYNC_IN_PROGRESS) {\
            return ASYNC_IN_PROGRESS;\
        }\
    } while (0)

#define block_on(state_field, fn, ...)\
    struct AsyncState_##fn state_field;\
    do {\
        async_state_##fn##_init(&state_field);\
        while (fn(&state_field __VA_OPT__(,) __VA_ARGS__) == ASYNC_IN_PROGRESS) {}\
    } while (0)

#endif /* INC_ASYNC_H_ */
