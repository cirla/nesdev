/**
 * Color in Motion
 *
 * http://timcheeseman.com/nesdev/
 */

#include <stdint.h>
#include <stddef.h>

#define TV_NTSC 1
#include "nes.h"
#include "reset.h"

#include "data.h"

#pragma bss-name(push, "ZEROPAGE")
uint8_t i;           // loop counter

// used by WritePPU method
uintptr_t       ppu_addr;      // destination PPU address
uint8_t const * ppu_data;      // pointer to data to copy to PPU
uint8_t         ppu_data_size; // # of bytes to copy to PPU
#pragma bss-name(pop)

#pragma bss-name(push, "OAM")
uint8_t sprites[256];
#pragma bss-name(pop)

void ResetScroll() {
    PPU_SCROLL = 0x00;
    PPU_SCROLL = 0x00;
}

void EnablePPU() {
    PPU_CTRL = PPUCTRL_NAMETABLE_0 | // use nametable 0
               PPUCTRL_BPATTERN_0  | // background uses pattern table 0
               PPUCTRL_NMI_ON      ; // enable NMIs

    PPU_MASK = PPUMASK_COLOR    | // show colors
               PPUMASK_BSHOW    | // show background
               PPUMASK_L8_BSHOW | // show background tiles in leftmost 8px
               PPUMASK_SSHOW    | // show sprites
               PPUMASK_L8_SSHOW ; // show sprites in leftmost 8px
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

    sprites[0] = 0x70;
    sprites[1] = (uint8_t) 'X';
    sprites[2] = 0x00;
    sprites[3] = 0x70;

    // turn on rendering
    ResetScroll();
    EnablePPU();

    while (1) {
        WaitFrame();
        ResetScroll();

        if (InputPort1 & BUTTON_UP) {
            sprites[0]--;
        }

        if (InputPort1 & BUTTON_DOWN) {
            sprites[0]++;
        }

        if (InputPort1 & BUTTON_LEFT) {
            sprites[3]--;
        }

        if (InputPort1 & BUTTON_RIGHT) {
            sprites[3]++;
        }
    };
};

