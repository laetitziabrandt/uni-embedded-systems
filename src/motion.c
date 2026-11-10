/*
 * Copyright (c) 2025 Hochschule Darmstadt
 *
 * This software is provided exclusively for the practical part of the
 * course 'Embedded Systems' at 'Darmstadt University of Applied Sciences
 * (Hochschule Darmstadt, h_da)'.
 *
 * Any use, distribution, or modification outside this context is prohibited
 * without explicit permission.
 *
 * AUTHORS: David Heiß, Manfred Pester, Jens-Peter Akelbein
 * FILE: motion.c
 * CONTENTS: implementation for all functions of motion module
 */

#include <constants.h>
#include <motion.h>
#include <state.h>

static float integral = 0.0f;
static struct Sample last_sample;
static bool has_last_sample = false;

void motion_loop(void)
{
    if (value.sonic.new && state.motion == MOTION_STATE_ON)
    {
        struct Sample current_sample = value.sonic;
        float setpoint = (float)SETPOINT;
        float error = setpoint - (float)current_sample.distance;

        if ((error < DEADZONE) && (error > -DEADZONE))
        {
            value.motion = 0.0f;
            integral = 0.0f;
            last_sample = current_sample;
            has_last_sample = true;
            return;
        }

        float p = error * KP;
        integral += error; // sum
        float i = KI * integral;

        float d = 0.0f;
        if (has_last_sample && (KD != 0.0f))
        {
            float error_last = setpoint - (float)last_sample.distance;
            float dt = (float)current_sample.time - (float)last_sample.time;
            if (dt > 0.0f)
            {
                d = KD * ((error - error_last) / dt);
            }
        }

        float pid = p + i + d;

        // Only slow down when approaching the setpoint; keep full reverse
        // authority when too close (especially with a small goal).
        float abs_error = (error >= 0.0f) ? error : -error;
        if (error > 0.0f && abs_error < SLOWDOWN_RANGE)
        {
            float scale = abs_error / (float)SLOWDOWN_RANGE;
            pid *= scale;
        }

        if (pid > MOTOR_MAX_RATE)
        {
            value.motion = MOTOR_MAX_RATE;
        }
        else if (pid < -MOTOR_MAX_RATE)
        {
            value.motion = -MOTOR_MAX_RATE;
        }
        else
        {
            value.motion = pid;
        }

        last_sample = current_sample;
        has_last_sample = true;
    }
}
