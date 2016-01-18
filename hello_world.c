/**
 * Hello, World!
 *
 * http://timcheeseman.com/nesdev/
 */

#include <stddef.h>
#include <stdint.h>

// define PPU register aliases
#define PPU_CTRL    *((uint8_t*)0x2000)
#define PPU_MASK    *((uint8_t*)0x2001)
#define PPU_STATUS  *((uint8_t*)0x2002)
#define PPU_SCROLL  *((uint8_t*)0x2005)
#define PPU_ADDRESS *((uint8_t*)0x2006)
#define PPU_DATA    *((uint8_t*)0x2007)

// define palette color aliases
#define COLOR_BLACK 0x0f
#define COLOR_WHITE 0x20

#pragma bss-name(push, "ZEROPAGE")
size_t i;
#pragma bss-name(pop)

const char TEXT[] = "Hello, World!";

const uint8_t PALETTE[] = {
    COLOR_BLACK,                           // background color
    COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, // background palette 0
};

/**
 * main() will be called at the end of the initialization code in reset.s.
 * Unlike C programs on a computer, it takes no arguments and returns no value.
 */
void main(void) {
    // load the palette data into PPU memory $3f00-$3f1f
    PPU_ADDRESS = 0x3f;
    PPU_ADDRESS = 0x00;
    for ( i = 0; i < sizeof(PALETTE); ++i ) {
        PPU_DATA = PALETTE[i];
    }

    // load the text sprites into the background (nametable 0)
    // nametable 0 is VRAM $2000-$23ff, so we'll choose an address in the
    // middle of the screen. The screen can hold a 32x30 grid of 8x8 sprites,
    // so an offset of 0x1ca (X: 10, Y:14) puts us around the middle vertically
    // and roughly centers our text horizontally.
    PPU_ADDRESS = 0x21;
    PPU_ADDRESS = 0xca;
    for ( i = 0; i < sizeof(TEXT); ++i ) {
        PPU_DATA = (uint8_t) TEXT[i];
    }

    // reset scroll location to top-left of screen
    PPU_SCROLL = 0x00;
    PPU_SCROLL = 0x00;

    // enable NMI and rendering
    PPU_CTRL = 0x80;
    PPU_MASK = 0x1e;

    // infinite loop
    while (1) {};
};

