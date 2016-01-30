/**
 * Color in Motion
 *
 * http://timcheeseman.com/nesdev/
 */

#include <stddef.h>
#include <stdint.h>

#include "nes.h"
#include "reset.h"

#pragma bss-name(push, "ZEROPAGE")
size_t i;
uintptr_t ppu_addr;
uint8_t attr_offset;
#pragma bss-name(pop)

const char TEXT[] = "Hello, World!";

const uint8_t PALETTE[] = {
    COLOR_BLUE,         // background color
    0, 0, COLOR_RED,    // background palette 0
    0,                  // ignored
    0, 0, COLOR_GREEN,  // background palette 1
    0,                  // ignored
    0, 0, COLOR_YELLOW, // background palette 2
    0,                  // ignored
    0, 0, COLOR_WHITE,  // background palette 3
};

const uint8_t ATTRIBUTES[] = {
    // layout 1
    0x00, // 00 00 00 00 or 0 0
          //                0 0
    0x90, // 10 01 00 00 or 0 0
          //                1 2
    0x40, // 01 00 00 00 or 0 0
          //                0 1
    0xe0, // 11 10 00 00 or 0 0
          //                2 3

    // layout 2
    0x80, // 10 00 00 00 or 0 0
          //                0 2
    0x40, // 01 00 00 00 or 0 0
          //                0 1
    0x20, // 00 10 00 00 or 0 0
          //                2 0
    0xd0, // 11 01 00 00 or 0 0
          //                1 3

    // layout 3
    0x40, // 01 00 00 00 or 0 0
          //                0 1
    0x20, // 00 10 00 00 or 0 0
          //                2 0
    0x90, // 10 01 00 00 or 0 0
          //                1 2
    0xc0, // 11 00 00 00 or 0 0
          //                0 3
};

void main(void) {
    PPU_ADDRESS = (uint8_t)(PPU_PALETTE >> 8);
    PPU_ADDRESS = (uint8_t)(PPU_PALETTE);
    for ( i = 0; i < sizeof(PALETTE); ++i ) {
        PPU_DATA = PALETTE[i];
    }

    ppu_addr = PPU_NAMETABLE_0 + 0x1ca;
    PPU_ADDRESS = (uint8_t)(ppu_addr >> 8);
    PPU_ADDRESS = (uint8_t)(ppu_addr);
    for ( i = 0; i < sizeof(TEXT); ++i ) {
        PPU_DATA = (uint8_t) TEXT[i];
    }

    ppu_addr = PPU_ATTRIB_TABLE_0 + 0x1a;
    PPU_ADDRESS = (uint8_t)(ppu_addr >> 8);
    PPU_ADDRESS = (uint8_t)(ppu_addr);
    for ( i = 0; i < 4; ++i ) {
        PPU_DATA = ATTRIBUTES[i];
    }

    // reset scroll location to top-left of screen
    PPU_SCROLL = 0x00;
    PPU_SCROLL = 0x00;

    // enable NMI and rendering
    PPU_CTRL = 0x80;
    PPU_MASK = 0x1e;

    attr_offset = 4;
    while (1) {
        // rotate colors every 30 frames, which is about every 0.5 seconds on NTSC
        if (FrameCount == 30) {
            PPU_ADDRESS = (uint8_t)(ppu_addr >> 8);
            PPU_ADDRESS = (uint8_t)(ppu_addr);
            for ( i = 0; i < 4; ++i ) {
                PPU_DATA = ATTRIBUTES[i + attr_offset];
            }

            attr_offset += 4;
            if (attr_offset == 12) {
                attr_offset = 0;
            }

            PPU_SCROLL = 0x00;
            PPU_SCROLL = 0x00;

            FrameCount = 0;
        }
    };
};

