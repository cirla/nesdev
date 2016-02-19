/**
 * nes.h
 *
 * http://timcheeseman.com/nesdev/
 *
 * Before including, #define either TV_NTSC or TV_PAL
 */

#ifndef NES_H_
#define NES_H_

#include <stdint.h>

// PPU registers
// see http://wiki.nesdev.com/w/index.php/PPU_registers
#define PPU_CTRL    *((uint8_t*) 0x2000)
#define PPU_MASK    *((uint8_t*) 0x2001)
#define PPU_STATUS  *((uint8_t*) 0x2002)
#define PPU_SCROLL  *((uint8_t*) 0x2005)
#define PPU_ADDRESS *((uint8_t*) 0x2006)
#define PPU_DATA    *((uint8_t*) 0x2007)

// PPU_CTRL flags
// see http://wiki.nesdev.com/w/index.php/PPU_registers#Controller_.28.242000.29_.3E_write
#define PPUCTRL_NAMETABLE_0 0x00 // use nametable 0
#define PPUCTRL_NAMETABLE_1 0x01 // use nametable 1
#define PPUCTRL_NAMETABLE_2 0x02 // use nametable 2
#define PPUCTRL_NAMETABLE_3 0x03 // use nametable 3
#define PPUCTRL_INC_1_HORIZ 0x00 // PPU_DATA increments 1 horizontally
#define PPUCTRL_INC_32_VERT 0x04 // PPU_DATA increments 32 vertically
#define PPUCTRL_SPATTERN_0  0x00 // sprite pattern table 0
#define PPUCTRL_SPATTERN_1  0x08 // sprite pattern table 1
#define PPUCTRL_BPATTERN_0  0x00 // background pattern table 0
#define PPUCTRL_BPATTERN_1  0x10 // background pattern table 1
#define PPUCTRL_SSIZE_8x8   0x00 // 8x8 sprite size
#define PPUCTRL_SSIZE_16x16 0x00 // 16x16 sprite size
#define PPUCTRL_NMI_OFF     0x00 // disable NMIs
#define PPUCTRL_NMI_ON      0x80 // enable NMIs

// PPU_MASK flags
// see http://wiki.nesdev.com/w/index.php/PPU_registers#Mask_.28.242001.29_.3E_write
#define PPUMASK_COLOR    0x00
#define PPUMASK_GRAY     0x01
#define PPUMASK_L8_BHIDE 0x00
#define PPUMASK_L8_BSHOW 0x02
#define PPUMASK_L8_SHIDE 0x00
#define PPUMASK_L8_SSHOW 0x04
#define PPUMASK_BHIDE    0x00
#define PPUMASK_BSHOW    0x08
#define PPUMASK_SHIDE    0x00
#define PPUMASK_SSHOW    0x10
#ifdef TV_NTSC
    #define PPUMASK_EM_RED   0x20
    #define PPUMASK_EM_GREEN 0x40
#else // TV_PAL
    #define PPUMASK_EM_RED   0x40
    #define PPUMASK_EM_GREEN 0x20
#endif
#define PPUMASK_EM_BLUE  0x80

// PPU memory addresses
// see http://wiki.nesdev.com/w/index.php/PPU_memory_map
// and http://wiki.nesdev.com/w/index.php/PPU_nametables
// and http://wiki.nesdev.com/w/index.php/PPU_attribute_tables
// and http://wiki.nesdev.com/w/index.php/PPU_palettes#Memory_Map
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
// see http://wiki.nesdev.com/w/index.php/PPU_palettes
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

// PPU resolution
// see http://wiki.nesdev.com/w/index.php/PPU_nametables
#define MIN_X        0
#define MAX_X      256
#define NUM_COLS    32
#ifdef TV_NTSC
  #define MIN_Y      8
  #define MAX_Y    231
  #define NUM_ROWS  28
  #define FIRST_ROW  1
  #define LAST_ROW  27
#else // TV_PAL
  #define MIN_Y      0
  #define MAX_Y    239
  #define NUM_ROWS  30
  #define FIRST_ROW  0
  #define LAST_ROW  29
#endif
#define NAMETABLE_OFFSET (NUM_COLS * FIRST_ROW)
#define SPRITE_HEIGHT    8
#define SPRITE_WIDTH     8

// PPU framerate
#ifdef TV_NTSC
  #define FRAMES_PER_SEC 60
#else // TV_PAL
  #define FRAMES_PER_SEC 50
#endif

// OAM sprite
typedef struct sprite {
    uint8_t y;
    uint8_t tile_index;
    uint8_t attributes;
    uint8_t x;
} sprite_t;

// standard controller buttons
#define BUTTON_RIGHT  0x01
#define BUTTON_LEFT   0x02
#define BUTTON_DOWN   0x04
#define BUTTON_UP     0x08
#define BUTTON_START  0x10
#define BUTTON_SELECT 0x20
#define BUTTON_B      0x40
#define BUTTON_A      0x80

#endif // NES_H_

