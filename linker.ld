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
 * FILE: linker.ld
 * CONTENTS: linker script
 */

MEMORY {
    FLASH (RX) : ORIGIN = 0X00080000, LENGTH = 512K
    RAM (RWX) : ORIGIN = 0x20070000, LENGTH = 64K + 32K
}

SECTIONS {
    .text : {
        KEEP(*(.vector*))
        *(.text*)
    } > FLASH

    .rodata : {
        *(.rodata*)
    } > FLASH

    .data : {
        __data_start = .;
        *(.data*)
        __data_end = .;
    } > RAM AT > FLASH
    __data_load = LOADADDR(.data);

    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss*)
        __bss_end = .;
    } > RAM

    __stack = ORIGIN(RAM) + LENGTH(RAM);
}

ENTRY(_start)
