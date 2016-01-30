/**
 * nes.h
 *
 * http://timcheeseman.com/nesdev/
 */

#ifndef NES_H_
#define NES_H_

// PPU registers
#define PPU_CTRL    *((uint8_t*)0x2000)
#define PPU_MASK    *((uint8_t*)0x2001)
#define PPU_STATUS  *((uint8_t*)0x2002)
#define PPU_SCROLL  *((uint8_t*)0x2005)
#define PPU_ADDRESS *((uint8_t*)0x2006)
#define PPU_DATA    *((uint8_t*)0x2007)

// PPU memory addresses
#define PPU_PATTERN_TABLE_0 0x0000 // pattern table 0
#define PPU_PATTERN_TABLE_1 0x1000 // pattern table 1
#define PPU_NAMETABLE_0     0x2000 // nametable 0
#define PPU_NAMETABLE_1     0x2400 // nametable 1
#define PPU_NAMETABLE_2     0x2800 // nametable 2
#define PPU_NAMETABLE_3     0x2c00 // nametable 3
#define PPU_ATTRIB_TABLE_0  0x23c0 // attribute table for nametable 0
#define PPU_ATTRIB_TABLE_1  0x27c0 // attribute table for nametable 1
#define PPU_ATTRIB_TABLE_2  0x2bc0 // attribute table for nametable 2
#define PPU_ATTRIB_TABLE_3  0x2fc0 // attribute table for nametable 3
#define PPU_PALETTE         0x3f00 // palette memory
#define PPU_PALLETE_BGC     0x3f00 // universal background color
#define PPU_PALETTE_BG_0    0x3f01 // background palette 0
#define PPU_PALETTE_BG_1    0x3f05 // background palette 1
#define PPU_PALETTE_BG_2    0x3f09 // background palette 2
#define PPU_PALETTE_BG_3    0x3f0d // background palette 3
#define PPU_PALETTE_SP_0    0x3f11 // sprite palette 0
#define PPU_PALETTE_SP_1    0x3f15 // sprite palette 1
#define PPU_PALETTE_SP_2    0x3f19 // sprite palette 2
#define PPU_PALETTE_SP_3    0x3f1d // sprite palette 3

// PPU palette colors
#define COLOR_AQUA    0x1c
#define COLOR_BLACK   0x0f
#define COLOR_BLUE    0x12
#define COLOR_BROWN   0x17
#define COLOR_DGRAY   0x00
#define COLOR_GREEN   0x1a
#define COLOR_LIME    0x2a
#define COLOR_LGRAY   0x10
#define COLOR_MAGENTA 0x24
#define COLOR_MAROON  0x06
#define COLOR_NAVY    0x02
#define COLOR_OLIVE   0x18
#define COLOR_PURPLE  0x14
#define COLOR_RED     0x16
#define COLOR_TEAL    0x2c
#define COLOR_WHITE   0x20
#define COLOR_YELLOW  0x28

// PPU sprite coordinates
#define MAX_X 32
#define MAX_Y 30

#endif // NES_H_

