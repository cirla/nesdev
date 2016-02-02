/**
 * Color in Motion
 *
 * http://timcheeseman.com/nesdev/
 */

#include <stddef.h>
#include <stdint.h>

#define TV_NTSC 1
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

#define ATTR_SIZE 4
#define ATTR_LEN  12
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

void ResetScroll() {
    PPU_SCROLL = 0x00;
    PPU_SCROLL = 0x00;
}

void EnablePPU() {
    PPU_CTRL = PPUCTRL_NAMETABLE_0 |
               PPUCTRL_INC_1_HORIZ |
               PPUCTRL_SPATTERN_0  |
               PPUCTRL_BPATTERN_0  |
               PPUCTRL_SSIZE_8x8   |
               PPUCTRL_NMI_ON      ;

    PPU_MASK = PPUMASK_COLOR    |
               PPUMASK_L8_BSHOW |
               PPUMASK_L8_SSHOW |
               PPUMASK_BSHOW    |
               PPUMASK_SSHOW    ;
}

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

    ResetScroll();
    EnablePPU();

    attr_offset = ATTR_SIZE;
    while (1) {
        // rotate colors every 30 frames, which is about every 0.5 seconds on NTSC
        if (FrameCount == 30) {
            PPU_ADDRESS = (uint8_t)(ppu_addr >> 8);
            PPU_ADDRESS = (uint8_t)(ppu_addr);
            for ( i = 0; i < 4; ++i ) {
                PPU_DATA = ATTRIBUTES[i + attr_offset];
            }

            attr_offset += ATTR_SIZE;
            if (attr_offset == ATTR_LEN) {
                attr_offset = 0;
            }

            ResetScroll();
            FrameCount = 0;
        }
    };
};

