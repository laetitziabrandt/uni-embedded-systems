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
 * FILE: sonic.h
 * CONTENTS: declaration of sonic driver using ultrasonic sensor HC-SR04
 */

#include <sonic.h>

#include <constants.h>
#include <sam.h>
#include <state.h>

// Measurements restarted with this frequency of trigger pulse
#define TRIGGER_FREQUENCY 10

// Sonic speed is about 343 m/s at 20 degrees Celsius
#define SONIC_SPEED 343

// 10 Hz trigger period at MCK/2 (42 MHz): 4_200_000 / 42_000_000 = 100 ms
#define TRIGGER_RC 4200000u
// Trigger pulse width: 42_000 / 42_000_000 = 1 ms (HC-SR04 needs >= 10 us)
#define TRIGGER_RB 42000u

static inline void trigger_enable(void)
{
    PIOC_PDR = PIOC_PDR_P29;
}

static inline void trigger_disable(void)
{
    PIOC_PER = PIOC_PER_P29;
}

static void trigger_init(void)
{
    TC2_CCR1 = TC2_CCR1_CLKDIS; // disable clock for timer
    PIOC_PER = PIOC_PER_P29;    // enable pin
    PIOC_OER = PIOC_OER_P29;    // set pin to output
    PIOC_CODR = PIOC_CODR_P29;  // clear output data register
    PIOC_ABSR |= PIOC_ABSR_P29; // select peripheral b for pin

    PMC_PCER1 |= PMC_PCER1_PID34; // enable timer clock

    TC2_CMR1 =
        TC2_CMR1_WAVE                                         // set timer to wave mode
        | TC2_CMR1_WAVSEL_UP_RC                               // enable up mode on rc compare
        | TC2_CMR1_EEVT_XC0                                   // set TIOB as output
        | TC2_CMR1_BCPB_CLEAR                                 // clear on compare with rb
        | TC2_CMR1_BCPC_SET                                   // set on compare with rc
        | TC2_CMR1_TCCLKS_TIMER_CLOCK1;                       // MCK/2 = 42 MHz

    // FIX: was MCK2/30 (~30 Hz / 33 ms). HC-SR04 needs ~60 ms between pulses for
    // long echoes; 10 Hz gives each measurement time to finish before the next trigger.
    TC2_RC1 = TRIGGER_RC;
    TC2_RB1 = TRIGGER_RB;

    TC2_CCR1 = TC2_CCR1_CLKEN | TC2_CCR1_SWTRG; // enable clock for timer
}

static void echo_init(void)
{
    TC2_CCR2 = TC2_CCR2_CLKDIS; // disable clock for timer
    PIOD_PER = PIOD_PER_P7;     // enable pin
    PIOD_ODR = PIOD_ODR_P7;     // set pin to input
    PIOD_CODR = PIOD_CODR_P7;
    PIOD_ABSR |= PIOD_ABSR_P7;  // select peripheral b for pin

    PMC_PCER1 |= PMC_PCER1_PID35; // enable timer clock

    TC2_CMR2 =
        TC2_CMR2_TCCLKS_TIMER_CLOCK1 // MCK/2 = 42 MHz (TIMER_CLOCK1, not MCK/32)
        | TC2_CMR2_LDRA_RISING       // capture echo rising edge into RA
        | TC2_CMR2_LDRB_FALLING      // capture echo falling edge into RB
        // FIX: LDBDIS (not LDBSTOP) — clock is disabled after RB load; echo_loop
        // restarts it via CLKEN | SWTRG after each completed pulse.
        | TC2_CMR2_LDBDIS;
    // FIX: removed CPCTRG — it reset the counter every MCK2/30 (~33 ms) and
    // aborted long echo pulses before the falling edge, so distant objects never
    // produced LDRBS and the distance froze at the last (closest) reading.

    PIOD_PDR = PIOD_PDR_P7;

    TC2_CCR2 = TC2_CCR2_CLKEN; // enable clock for timer
    TC2_CCR2 = TC2_CCR2_SWTRG;
}

void sonic_init(void)
{
    trigger_init();
    echo_init();
}

static void trigger_loop(void)
{
    if (state.sonic == SONIC_STATE_OFF)
        trigger_disable();
    else
        trigger_enable();
}

static void echo_loop(void)
{
    if (state.sonic == SONIC_STATE_OFF)
        return;

    // Reading SR clears LDRBS; one read per loop iteration is enough for polling.
    unsigned int SR = TC2_SR2;

    if (SR & TC2_SR2_LDRBS)
    {
        unsigned int RA = TC2_RA2;
        unsigned int RB = TC2_RB2;
        unsigned int diff = RB - RA;

        // Timer ticks at MCK/2; echo high time is round-trip delay.
        // distance = c * (diff / (MCK/2)) / 2 = c * diff / MCK  (in metres)
        float time = diff / (double)MCK;
        float distance = (float)SONIC_SPEED * time;

        value.sonic.distance = distance * 1000.0f;
        value.sonic.new = true;

        // Restart echo capture for the next pulse (required after LDBDIS).
        TC2_CCR2 = TC2_CCR2_CLKEN | TC2_CCR2_SWTRG;
    }
}

void sonic_loop(void)
{
    trigger_loop();
    echo_loop();
}
