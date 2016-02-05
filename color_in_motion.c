/**
 * Color in Motion
 *
 * http://timcheeseman.com/nesdev/
 */

#include <stdint.h>

#define TV_NTSC 1
#include "nes.h"
#include "reset.h"

#include "data.h"

#pragma bss-name(push, "ZEROPAGE")
uint8_t         i;

// used by WritePPU method
uintptr_t       ppu_addr;      // destination PPU address
uint8_t const * ppu_data;      // pointer to data to copy to PPU
uint8_t         ppu_data_size; // # of bytes to copy to PPU

uint8_t         attr_offset;   // offset into ATTRIBUTES
#pragma bss-name(pop)

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
    ppu_addr = PPU_NAMETABLE_0 + TEXT_OFFSET;
    ppu_data = (uint8_t const *) TEXT;
    ppu_data_size = sizeof(TEXT);
    WritePPU();

    // write attributes
    ppu_addr = PPU_ATTRIB_TABLE_0 + ATTR_OFFSET;
    ppu_data = ATTRIBUTES;
    ppu_data_size = ATTR_SIZE;
    WritePPU();

    // turn on rendering
    ResetScroll();
    EnablePPU();

    attr_offset = ATTR_SIZE;
    while (1) {
        // rotate colors every 30 frames, which is about every 0.5 seconds on NTSC
        if (FrameCount == 30) {
            // write attributes
            ppu_data = ATTRIBUTES + attr_offset;
            WritePPU();

            // rotate attributes
            attr_offset += ATTR_SIZE;
            if (attr_offset == sizeof(ATTRIBUTES)) {
                attr_offset = 0;
            }

            ResetScroll();
            FrameCount = 0;
        }
    };
};

