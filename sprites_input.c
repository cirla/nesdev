/**
 * Sprites and Input
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
uint8_t i; // loop counter
uint8_t j; // loop counter

// used by WritePPU method
uintptr_t       ppu_addr;      // destination PPU address
uint8_t const * ppu_data;      // pointer to data to copy to PPU
uint8_t         ppu_data_size; // # of bytes to copy to PPU
#pragma bss-name(pop)

#pragma bss-name(push, "OAM")
sprite_t player;
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

void DrawBackground() {
    PPU_ADDRESS = (uint8_t)((PPU_NAMETABLE_0 + NAMETABLE_OFFSET) >> 8);
    PPU_ADDRESS = (uint8_t)(PPU_NAMETABLE_0 + NAMETABLE_OFFSET);

    // draw top
    PPU_DATA = BORDER_TL;
    for(i = 0; i < (NUM_COLS - 2); ++i) {
        PPU_DATA = BORDER_T;
    }
    PPU_DATA = BORDER_TR;

    // draw sides
    for(i = 0; i < (NUM_ROWS - 2); ++i) {
        PPU_DATA = BORDER_L;
        for(j = 0; j < (NUM_COLS - 2); ++j) {
            PPU_DATA = BLANK_TILE;
        }
        PPU_DATA = BORDER_R;
    }

    // draw bottom
    PPU_DATA = BORDER_BL;
    for(i = 0; i < (NUM_COLS - 2); ++i) {
        PPU_DATA = BORDER_B;
    }
    PPU_DATA = BORDER_BR;
}

void main(void) {
    // write palettes
    ppu_addr = PPU_PALETTE;
    ppu_data = PALETTES;
    ppu_data_size = sizeof(PALETTES);
    WritePPU();

    // draw background
    DrawBackground();

    // initialize player sprite
    player.x = (MAX_X / 2) - (SPRITE_WIDTH / 2);
    player.y = (MAX_Y / 2) - (SPRITE_HEIGHT / 2);
    player.tile_index = SPRITE_PLAYER;

    // turn on rendering
    ResetScroll();
    EnablePPU();

    while (1) {
        WaitFrame();
        ResetScroll();

        if (InputPort1 & BUTTON_UP) {
            if (player.y > MIN_Y + SPRITE_HEIGHT) {
                --player.y;
            }
        }

        if (InputPort1 & BUTTON_DOWN) {
            if (player.y < MAX_Y - (2 * SPRITE_HEIGHT)) {
                ++player.y;
            }
        }

        if (InputPort1 & BUTTON_LEFT) {
            if (player.x > MIN_X + SPRITE_WIDTH) {
                --player.x;
            }
        }

        if (InputPort1 & BUTTON_RIGHT) {
            if (player.x < MAX_X - (2 * SPRITE_WIDTH)) {
                ++player.x;
            }
        }
    };
};

