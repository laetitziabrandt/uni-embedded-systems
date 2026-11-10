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
 * FILE: drive.h
 * CONTENTS: implementation for all functions for PWM motor drive
 */

#include <drive.h>

#include <constants.h>
#include <state.h>
#include <sam.h>

static unsigned int calculate_duty_cycle(double motion_value)
{
    if (motion_value < 0.0)
    {
        motion_value = -motion_value; // make positive
    }

    if (motion_value > MOTOR_MAX_RATE)
    {
        motion_value = MOTOR_MAX_RATE; // clamp to max
    }

    // if motion_value 0 then duty cycle is min rate of 10%
    double duty_percent = MOTOR_MIN_RATE + ((MOTOR_MAX_RATE - MOTOR_MIN_RATE) * motion_value / MOTOR_MAX_RATE);
    return (unsigned int)((double)TC2_RC0 * (duty_percent / MOTOR_MAX_RATE));
}

void drive_init()
{
    // disable motor and enable pins for direction
    PIOC_PER |= PIOC_PER_P25 | PIOC_PER_P23 | PIOC_PER_P24;

    // turn on multiplexing B on PIOC
    PIOC_ABSR |= PIOC_ABSR_P25;

    // set as output
    PIOC_OER |= PIOC_OER_P23 | PIOC_OER_P24 | PIOC_OER_P25;
    // set output to 0
    PIOC_CODR |= PIOC_CODR_P23 | PIOC_CODR_P24 | PIOC_CODR_P25;

    // enable PIOC
    PMC_PCER0 |= PIOC_ID;
    // enable TCC 6 and PWM?
    PMC_PCER1 |= PMC_PCER1_PID33 | PMC_PCER1_PID36;

    // disable timer
    TC2_CCR0 = TC2_CCR0_CLKDIS;

    // configure pwm signal
    TC2_CMR0 |= TC2_CMR0_WAVE            // enable wave mode
        | TC2_CMR0_TCCLKS_TIMER_CLOCK1  // prescaler 2
        | TC2_CMR0_WAVSEL_UP_RC         // count up and reset when rc compare
        | TC2_CMR0_ACPA_CLEAR           // set to low when TIOA cmpares to RA
        | TC2_CMR0_ACPC_SET             // set to high when TIOA comapres to RC
        | TC2_CMR0_EEVT_XC0;

    // set RA and RC
    TC2_RC0 = MCK2/MOTOR_MAX_RATE;
    TC2_RA0 = TC2_RC0/2; // 50% duty cycle
    
    // enable timer
    TC2_CCR0 = TC2_CCR0_CLKEN | TC2_CCR0_SWTRG;
}

void drive_forward()
{
    PIOC_CODR |= PIOC_CODR_P23;
    PIOC_SODR |= PIOC_SODR_P24;
}

void drive_backward()
{
    PIOC_SODR |= PIOC_SODR_P23;
    PIOC_CODR |= PIOC_CODR_P24;
}

void drive_stop()
{
    PIOC_CODR |= PIOC_CODR_P23 | PIOC_CODR_P24;
}

void drive_loop()
{
    if (state.sonic == SONIC_STATE_ON && state.motion == MOTION_STATE_ON) // jic ensure sonic is on
    {
        PIOC_PDR = PIOC_PDR_P25;

        if (value.motion > 0.0)
        {
            drive_forward();
            TC2_RA0 = calculate_duty_cycle(value.motion);
        }
        else if (value.motion < 0.0)
        {
            drive_backward();
            TC2_RA0 = calculate_duty_cycle(value.motion);
        }
        else
        {
            drive_stop();
            TC2_RA0 = 0;
        }
    }
    else
    {
        PIOC_PER = PIOC_PER_P25;
    }
}
