/**
 * data.h
 *
 * http://timcheeseman.com/nesdev/
 */

#ifndef DATA_H_
#define DATA_H_

#include <stdint.h>

#include "nes.h"

uint8_t const PALETTES[] = {
    COLOR_BLUE,         // background color

    0, 0, COLOR_RED,    // background palette 0
    0,                  // ignored
    0, 0, COLOR_GREEN,  // background palette 1
    0,                  // ignored
    0, 0, COLOR_YELLOW, // background palette 2
    0,                  // ignored
    0, 0, COLOR_WHITE,  // background palette 3

    COLOR_BLUE,         // background color (mirror)

    0, 0, COLOR_RED,    // sprite palette 0
    0,                  // ignored
    0, 0, COLOR_GREEN,  // sprite palette 1
    0,                  // ignored
    0, 0, COLOR_YELLOW, // sprite palette 2
    0,                  // ignored
    0, 0, COLOR_WHITE,  // sprite palette 3
};

#endif // DATA_H_

