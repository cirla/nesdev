/**
 * Color in Motion
 *
 * http://timcheeseman.com/nesdev/
 */

#include <stdint.h>

#define TV_NTSC 1
#include "nes.h"
#include "reset.h"

#pragma bss-name(push, "ZEROPAGE")
uint8_t         i;

uintptr_t       ppu_addr;
uint8_t const * ppu_data;
uint8_t         ppu_data_size;

uint8_t         attr_offset;
#pragma bss-name(pop)

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
#define ATTR_LEN  12
uint8_t const ATTRIBUTES[] = {
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
    PPU_CTRL = PPUCTRL_NAMETABLE_0 | // use nametable 0
               PPUCTRL_BPATTERN_0  | // background uses pattern table 0
               PPUCTRL_NMI_ON      ; // enable NMIs

    PPU_MASK = PPUMASK_COLOR | // show colors
               PPUMASK_BSHOW ; // show background
}

void WritePPU() {
    PPU_ADDRESS = (uint8_t)(ppu_addr >> 8);
    PPU_ADDRESS = (uint8_t)(ppu_addr);
    for ( i = 0; i < ppu_data_size; ++i ) {
        PPU_DATA = ppu_data[i];
    }
}

void main(void) {
    // write palettes
    ppu_addr = PPU_PALETTE;
    ppu_data = PALETTES;
    ppu_data_size = sizeof(PALETTES);
    WritePPU();


    // write background tiles
    ppu_addr = PPU_NAMETABLE_0 + 0x1ca;
    ppu_data = (uint8_t const *) TEXT;
    ppu_data_size = sizeof(TEXT);
    WritePPU();

    // write attributes
    ppu_addr = PPU_ATTRIB_TABLE_0 + 0x1a;
    ppu_data = ATTRIBUTES;
    ppu_data_size = sizeof(ATTRIBUTES);
    WritePPU();

    ResetScroll();
    EnablePPU();

    attr_offset = ATTR_SIZE;
    ppu_data_size = ATTR_SIZE;
    while (1) {
        // rotate colors every 30 frames, which is about every 0.5 seconds on NTSC
        if (FrameCount == 30) {
            // write attributes
            ppu_data = ATTRIBUTES + attr_offset;
            WritePPU();

            // rotate attributes
            attr_offset += ATTR_SIZE;
            if (attr_offset == ATTR_LEN) {
                attr_offset = 0;
            }

            ResetScroll();
            FrameCount = 0;
        }
    };
};

