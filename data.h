/**
 * data.h
 *
 * http://timcheeseman.com/nesdev/
 */

#ifndef DATA_H_
#define DATA_H_

#include <stdint.h>

#include "nes.h"

#define TEXT_X      10
#define TEXT_Y      14
#define TEXT_OFFSET (TEXT_Y * NUM_COLS + TEXT_X)
char const TEXT[] = "Hello, World!";

uint8_t const PALETTES[] = {
    COLOR_BLUE,         // background color
    0, 0, COLOR_RED,    // background palette 0
    0,                  // ignored
    0, 0, COLOR_GREEN,  // background palette 1
    0,                  // ignored
    0, 0, COLOR_YELLOW, // background palette 2
    0,                  // ignored
    0, 0, COLOR_WHITE,  // background palette 3
};

#define ATTR_SIZE 4
#define ATTR_OFFSET ((TEXT_Y / 4) * (NUM_COLS / 4) + (TEXT_X / 4))
uint8_t const ATTRIBUTES[] = {
    // layout 1 - 0120123
    0x00, // 00 00 00 00 or 0 0
          //                0 0
    0x90, // 10 01 00 00 or 0 0
          //                1 2
    0x40, // 01 00 00 00 or 0 0
          //                0 1
    0xe0, // 11 10 00 00 or 0 0
          //                2 3

    // layout 2 - 2012013
    0x80, // 10 00 00 00 or 0 0
          //                0 2
    0x40, // 01 00 00 00 or 0 0
          //                0 1
    0x20, // 00 10 00 00 or 0 0
          //                2 0
    0xd0, // 11 01 00 00 or 0 0
          //                1 3

    // layout 3 - 1201203
    0x40, // 01 00 00 00 or 0 0
          //                0 1
    0x20, // 00 10 00 00 or 0 0
          //                2 0
    0x90, // 10 01 00 00 or 0 0
          //                1 2
    0xc0, // 11 00 00 00 or 0 0
          //                0 3
};

#endif // DATA_H_

