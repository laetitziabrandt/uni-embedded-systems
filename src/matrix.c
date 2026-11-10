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
 * FILE: matrix.c
 * CONTENTS: implementation for all functions driving I2C LED matrix
 */

#include "matrix.h"

#include <cm3.h>
#include <sam.h>
#include <state.h>
#include <stdbool.h>

static unsigned char buffer[] = {[0 ... 16] = 0};
static unsigned int decimal[] = {
    0b00000000,
    0b10000000, 0b11000000,
    0b11100000, 0b11110000,
    0b11111000, 0b11111100,
    0b11111110, 0b11111111};

static bool matrix_ready()
{
    return (TWI1_SR & TWI1_SR_TXCOMP);
}

static bool matrix_write(const unsigned char *buf, unsigned int n)
{
    if (!matrix_ready())
        return false;

    TWI1_TPR = (unsigned int)buf;
    TWI1_TCR = n;
    // stop sending
    TWI1_CR = TWI1_CR_STOP;

    return true;
}

static bool matrix_blinking(bool blink)
{
    if (!matrix_ready())
        return false;

    unsigned char init_sequence[] = {
        0x21,                // internal system clock enable
        0xA0,                // row output pin set
        0xE0,                // dimming set
        blink ? 0x83 : 0x81, // blinking set
    };

    // send sequence
    for (unsigned int i = 0; i < sizeof(init_sequence); i++)
        while (!matrix_write(init_sequence + i, 1))
            ;
    return true;
}

void matrix_clear()
{
    for (int i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = decimal[0];
    }
}

static bool matrix_distance(int dist)
{
    if (!matrix_ready())
        return false;

    static bool blink = false;
    // start blinking when value is negative
    if (!blink && dist <= 0)
        while (!matrix_blinking(blink = true))
            ;
    else if (blink && dist > 0)
        while (!matrix_blinking(blink = false))
            ;

    // invert distance if negative
    if (dist <= 0)
        dist = -dist;

    show_led_nr(dist);
    matrix_write(buffer, sizeof(buffer));
    return true;
}

void matrix_init(void)
{
    // enable TWI1 -> we use TWI1 because the matrix is connected to TWI1 and not TWI0
    PMC_PCER0 = PMC_PCER0_PID23;
    // enable peripherals
    PIOB_PER = PIOB_PER_P12 | PIOB_PER_P13;
    // enable SDA and SCL as outputs
    PIOB_OER = PIOB_OER_P12 | PIOB_OER_P13;
    // clear SDA as output
    PIOB_CODR = PIOB_CODR_P12;
    // set SCL as output
    PIOB_SODR = PIOB_SODR_P13;

    // disable dma
    TWI1_PTCR = TWI1_PTCR_RXTDIS | TWI1_PTCR_TXTDIS;
    // reset TWI
    TWI1_CR = TWI1_CR_SWRST;

    // configure waveform
    // normal mode
    // TWI1_CWGR = TWI1_CWGR_CKDIV(5) | TWI1_CWGR_CHDIV(13) | TWI1_CWGR_CLDIV(13);
    // fast mode
    TWI1_CWGR = TWI1_CWGR_CKDIV(3) | TWI1_CWGR_CHDIV(13) | TWI1_CWGR_CLDIV(13);
    // set master mode values
    TWI1_MMR = TWI1_MMR_IADRSZ_NONE | TWI1_MMR_DADR(0x70);
    // enable master mode
    TWI1_CR = TWI1_CR_MSEN;

    // reset pointer
    TWI1_TPR = 0;
    // reset counter
    TWI1_TCR = 0;
    // enable receive and transmit. why receive(ACK)?
    // TWI1_PTCR = TWI1_PTCR_RXTEN | TWI1_PTCR_TXTEN;
    TWI1_PTCR = TWI1_PTCR_TXTEN;

    // turn on port multiplexing
    PIOB_ABSR &= ~(PIOB_ABSR_P12 | PIOB_ABSR_P13);

    // disable peripheral pins
    PIOB_PDR = PIOB_PDR_P12 | PIOB_PDR_P13;

    unsigned char init_sequence[] = {
        0x21, // internal system clock enable
        0xA0, // row output pin set
        0xE0, // dimming set
        0x81, // blinking set
    };

    for (unsigned int i = 0; i < sizeof(init_sequence); i++)
        while (!matrix_write(init_sequence + i, 1))
            ;

    while (!matrix_write(buffer, sizeof(buffer)))
        ;
}

void show_led_nr(int count)
{
    int full_rows = count / 8;
    int leftover_leds = count % 8;
    int temp = 0;

    for (unsigned int i = 0; i < full_rows; i++)
    {
        if (i == 0)
        {
            buffer[i + 1] = decimal[8];
        }
        temp = ((i * 2) + 1);
        buffer[temp] = decimal[8];
    }

    int index = 0;
    if (full_rows == 0)
    {
        index = 1;
    }
    else
    {
        index = temp + 2; // e.g. if full_rows = 3 then 3+6 = 9, so the next row
    }

    buffer[index] = decimal[leftover_leds];
}

void matrix_loop(void)
{
    if (state.sonic == SONIC_STATE_OFF)
    {
        buffer[1] = 0xFF; // Zeile 1;
        buffer[3] = 0xFF;
        buffer[5] = 0x06;
        buffer[7] = 0x0C;
        buffer[9] = 0x18;
        buffer[11] = 0x30;
        buffer[13] = 0x60;
        buffer[15] = 0xC0;
    }
    else
    {
        if (value.sonic.new)
        {
            matrix_clear();
            matrix_write(buffer, sizeof(buffer));
            while (!matrix_distance((int)value.sonic.distance - (int)value.goal))
                ;
            value.sonic.new = false;
        }
    }

    matrix_write(buffer, sizeof(buffer));
}