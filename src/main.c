#include <stdint.h>
#include <stddef.h>

#define TV_NTSC 1
#include "famitone2.h"
#include "nes.h"
#include "reset.h"
#include "rle.h"

#include "data.h"

typedef enum state {
    STATE_TITLE,
    STATE_LEVEL,
    STATE_FADE,
    STATE_CREDITS
} state_t;


typedef enum collision {
    COLLISION_NONE = 0,
    COLLISION_RING,
    COLLISION_BRIDE
} collision_t;


#define FADE_MSG_ROW 12
#define FADE_MSG_COL 14
#define FADE_MSG_LEN  4
const char FADE_MSG[] = "YES!";


#define CREDITS_MSGS    3
#define CREDITS_MSG_LEN 4
#define CREDITS_LEN     (CREDITS_MSGS * CREDITS_MSG_LEN)
#define CREDITS_WIDTH   8
#define N_TEXT_SPRITES  (CREDITS_MSG_LEN * CREDITS_WIDTH)
const char CREDITS_TEXT[CREDITS_LEN][CREDITS_WIDTH + 1] = {
  "CONGRATS",
  "LILIBET ",
  "  AND   ",
  "PHILIP! ",

  "MAY YOUR",
  "LIFE BE ",
  "FULL OF ",
  " LOVE.  ",

  "NOW YOUR",
  "  TRUE  ",
  " QUEST  ",
  "BEGINS! ",
};

#define CREDITS_MIN_X  96
#define CREDITS_MIN_Y 100
#define CREDITS_MAX_Y 200

#define CREDITS_DATE_ROW 27
#define CREDITS_DATE_COL 10
#define CREDITS_DATE_LEN 12
const char CREDITS_DATE[] = "NOV. 20 1947";


#pragma bss-name(push, "ZEROPAGE")
uint8_t i;             // loop counter/temp
uint8_t j;             // loop counter/temp
state_t state;
const uint8_t * bg;    // background data
uint8_t pattern_table; // 0 or 1
uint8_t credits_msg;   // index into CREDITS_TEXT

player_t player;
uint8_t const * player_sprite_data;

uint8_t fade_palette[sizeof(PAL_LEVEL)];
uint8_t fade_count;
uint8_t text_locs[N_TEXT_SPRITES];

// used by WritePPU method
uintptr_t       ppu_addr;      // destination PPU address
uint8_t const * ppu_data;      // pointer to data to copy to PPU
uint8_t         ppu_data_size; // # of bytes to copy to PPU
#pragma bss-name(pop)


// sprites
#define PLAYER_START_X  20
#define PLAYER_START_Y 180
#define BRIDE_X        208
#define BRIDE_Y         48
#define PERSON_DIM      32
#define RING_START_X   120
#define RING_START_Y   112
#define RING_INV_X     236
#define RING_INV_Y      11
#define RING_DIM        16


#pragma bss-name(push, "OAM")
sprite_t player_sprites[16];           // 4x4 grid
sprite_t ring_sprites[4];              // 2x2 grid
sprite_t text_sprites[N_TEXT_SPRITES]; // 4x8 grid
#pragma bss-name(pop)


void ResetScroll() {
    PPU_SCROLL = 0x00;
    PPU_SCROLL = 0x00;
}


void EnablePPU() {
    i = PPUCTRL_NAMETABLE_0 | // use nametable 0
        PPUCTRL_NMI_ON      ; // enable NMIs

    if(pattern_table == 0) {
        i |= (PPUCTRL_BPATTERN_0 | PPUCTRL_SPATTERN_0);
    } else {
        i |= (PPUCTRL_BPATTERN_1 | PPUCTRL_SPATTERN_1);
    }

    PPU_CTRL = i;
    PPU_MASK = PPUMASK_COLOR    | // show colors
               PPUMASK_BSHOW    | // show background
               PPUMASK_L8_BSHOW | // show background tiles in leftmost 8px
               PPUMASK_SSHOW    | // show sprites
               PPUMASK_L8_SSHOW ; // show sprites in leftmost 8px
}


void DisablePPU() {
    PPU_CTRL = 0x00;
    PPU_MASK = 0x00;
}


void WritePPU() {
    PPU_ADDRESS = (uint8_t)(ppu_addr >> 8);
    PPU_ADDRESS = (uint8_t)(ppu_addr);
    for ( i = 0; i < ppu_data_size; ++i ) {
        PPU_DATA = ppu_data[i];
    }
}


void DrawBackground() {
    PPU_ADDRESS = (uint8_t)(PPU_NAMETABLE_0 >> 8);
    PPU_ADDRESS = (uint8_t)(PPU_NAMETABLE_0);
    UnRLE(bg);
}

void InitTitle() {
    DisablePPU();

    state = STATE_TITLE;
    pattern_table = 0;

    // write palettes
    ppu_addr = PPU_PALETTE;
    ppu_data = PAL_TITLE;
    ppu_data_size = sizeof(PAL_TITLE);
    WritePPU();

    // draw background
    bg = BG_TITLE;
    DrawBackground();

    // play music
    FamiToneMusicPlay(SONG_TITLE);

    WaitVBlank();
    EnablePPU();
}


void InitLevel() {
    DisablePPU();

    state = STATE_LEVEL;
    pattern_table = 1;

    // write palettes
    ppu_addr = PPU_PALETTE;
    ppu_data = PAL_LEVEL;
    ppu_data_size = sizeof(PAL_LEVEL);
    WritePPU();

    // draw background
    bg = BG_LEVEL;
    DrawBackground();

    // play music
    FamiToneMusicPlay(SONG_LEVEL);

    // initialize player
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player_sprite_data = GROOM_FRONT[0];

    for(i = 0; i < 4; ++i) {
        for(j = 0; j < 4; ++j) {
            player_sprites[i * 4 + j].x = player.x + j * SPRITE_WIDTH;
            player_sprites[i * 4 + j].y = player.y + i * SPRITE_HEIGHT;
            player_sprites[i * 4 + j].tile_index = player_sprite_data[i * 4 + j];
            player_sprites[i * 4 + j].attributes = i < 2 ? 0x01 : 0x02;
        }
    }

    for(i = 0; i < 2; ++i) {
        for(j = 0; j < 2; ++j) {
            ring_sprites[i * 2 + j].x = RING_START_X + j * SPRITE_WIDTH;
            ring_sprites[i * 2 + j].y = RING_START_Y + i * SPRITE_HEIGHT;
            ring_sprites[i * 2 + j].tile_index = RING_SPRITES[0][i * 2 + j];
        }
    }

    WaitVBlank();
    EnablePPU();
}


void InitFade() {
    FamiToneMusicPlay(SONG_CREDITS);
    state = STATE_FADE;
    FrameCount = 0;
    fade_count = 0;

    for ( i = 0; i < sizeof(PAL_LEVEL); ++i ) {
        fade_palette[i] = PAL_LEVEL[i];
    }

    DisablePPU();
    ResetScroll();

    ppu_addr = PPU_NAMETABLE_0 + (FADE_MSG_ROW * NUM_COLS) + FADE_MSG_COL;
    PPU_ADDRESS = (uint8_t)(ppu_addr >> 8);
    PPU_ADDRESS = (uint8_t)(ppu_addr);
    for (i = 0; i < FADE_MSG_LEN; ++i) {
        PPU_DATA = CHR_TO_IDX[FADE_MSG[i]];
    }

    WaitVBlank();
    EnablePPU();
}


void InitCreditsSprites() {
    for(i = 0; i < CREDITS_MSG_LEN; ++i) {
        for(j = 0; j < CREDITS_WIDTH; ++j) {
            #define OFFSET (i * CREDITS_WIDTH + j)
            text_locs[OFFSET] = CREDITS_MAX_Y + SPRITE_HEIGHT + SPRITE_HEIGHT * i;
            text_sprites[OFFSET].x = CREDITS_MIN_X + SPRITE_WIDTH * j;
            text_sprites[OFFSET].tile_index =
                CHR_TO_IDX[
                    CREDITS_TEXT[0][
                        (credits_msg * CREDITS_WIDTH * CREDITS_MSG_LEN) + // msg chunk
                        (i * CREDITS_WIDTH) + i +                         // null terminators
                        (credits_msg * CREDITS_MSG_LEN) +                 // msg line
                        j]];                                              // line char
            #undef OFFSET
        }
    }
}


void FadeStep() {
    for ( i = 0; i < sizeof(PAL_LEVEL); ++i ) {
        if (fade_palette[i] <= 0x10) {
            fade_palette[i] = COLOR_BLACK;
        } else if(fade_palette[i] != COLOR_BLACK) {
            fade_palette[i] -= 0x10;
        }
    }

    DisablePPU();

    ppu_addr = PPU_PALETTE;
    ppu_data = fade_palette;
    ppu_data_size = sizeof(PAL_LEVEL);
    WritePPU();

    WaitVBlank();
    EnablePPU();
}


void InitCredits() {
    DisablePPU();

    // hide sprites
    for(i = 0; i < 16; i++) {
        player_sprites[i].y = SPRITE_OFFSCREEN_Y;
    }
    for(i = 0; i < 4; i++) {
        ring_sprites[i].y = SPRITE_OFFSCREEN_Y;
    }

    state = STATE_CREDITS;
    pattern_table = 1;
    credits_msg = 0;

    // write palettes
    ppu_addr = PPU_PALETTE;
    ppu_data = PAL_CREDITS;
    ppu_data_size = sizeof(PAL_CREDITS);
    WritePPU();

    // draw background
    bg = BG_CREDITS;
    DrawBackground();

    InitCreditsSprites();
    FrameCount = 0;

    WaitVBlank();
    EnablePPU();
}


collision_t DetectCollision() {
    if (player.x < BRIDE_X + PERSON_DIM &&
        player.x + PERSON_DIM > BRIDE_X &&
        player.y < BRIDE_Y + PERSON_DIM &&
        PERSON_DIM + player.y > BRIDE_Y) {
        return COLLISION_BRIDE;
    }

    if (ring_sprites[0].x == RING_START_X &&
        player.x < RING_START_X + RING_DIM &&
        player.x + PERSON_DIM > RING_START_X &&
        player.y < RING_START_Y + RING_DIM &&
        PERSON_DIM + player.y > RING_START_Y) {
        return COLLISION_RING;
    }

    return COLLISION_NONE;
}


void MovePlayer() {
    if (InputPort1 & BUTTON_UP) {
        if (player.y > MIN_Y + SPRITE_HEIGHT * 4 + SPRITE_HEIGHT) {
            --player.y;
            if(DetectCollision()) { ++player.y; }
            player_sprite_data = GROOM_BACK[0];
        }
    }

    if (InputPort1 & BUTTON_DOWN) {
        if (player.y < MAX_Y - (4 * SPRITE_HEIGHT) - SPRITE_HEIGHT) {
            ++player.y;
            if(DetectCollision()) { --player.y; }
            player_sprite_data = GROOM_FRONT[0];
        }
    }

    if (InputPort1 & BUTTON_LEFT) {
        if (player.x > MIN_X + SPRITE_WIDTH) {
            --player.x;
            if(DetectCollision()) { ++player.x; }
            player_sprite_data = GROOM_LEFT[0];
        }
    }

    if (InputPort1 & BUTTON_RIGHT) {
        if (player.x < MAX_X - (4 * SPRITE_WIDTH) - SPRITE_WIDTH) {
            ++player.x;
            if(DetectCollision()) { --player.x; }
            player_sprite_data = GROOM_RIGHT[0];
        }
    }
}


void PlayerAction() {
    if(ring_sprites[0].x == RING_START_X) {
        i = 0;
        if(player_sprite_data == GROOM_FRONT[0]) {
            ++player.y;
            if(DetectCollision() == COLLISION_RING) { i = 1; }
            --player.y;
        }
        else if(player_sprite_data == GROOM_BACK[0]) {
            --player.y;
            if(DetectCollision() == COLLISION_RING) { i = 1; }
            ++player.y;
        }
        else if(player_sprite_data == GROOM_LEFT[0]) {
            --player.x;
            if(DetectCollision() == COLLISION_RING) { i = 1; }
            ++player.x;
        }
        else if(player_sprite_data == GROOM_RIGHT[0]) {
            ++player.x;
            if(DetectCollision() == COLLISION_RING) { i = 1; }
            --player.x;
        }

        if(i) {
            FamiToneSfxPlay(SFX_RING);
            for(i = 0; i < 2; ++i) {
                for(j = 0; j < 2; ++j) {
                    ring_sprites[i * 2 + j].x = RING_INV_X + j * SPRITE_WIDTH;
                    ring_sprites[i * 2 + j].y = RING_INV_Y + i * SPRITE_HEIGHT;
                }
            }
        }
    } else if(ring_sprites[0].x == RING_INV_X) {
        i = 0;
        if(player_sprite_data == GROOM_BACK[0]) {
            --player.y;
            if(DetectCollision() == COLLISION_BRIDE) { i = 1; }
            ++player.y;
        }
        else if(player_sprite_data == GROOM_RIGHT[0]) {
            ++player.x;
            if(DetectCollision() == COLLISION_BRIDE) { i = 1; }
            --player.x;
        }

         if(i) {
            FamiToneSfxPlay(SFX_RING);
            for(i = 0; i < 2; ++i) {
                for(j = 0; j < 2; ++j) {
                    ring_sprites[i * 2 + j].x = BRIDE_X + j * SPRITE_WIDTH + 5;
                    ring_sprites[i * 2 + j].y = BRIDE_Y + i * SPRITE_HEIGHT + 14;
                }
            }
            InitFade();
        }
    }
}


void HandleInput() {
    switch(state) {
        case STATE_TITLE:
            if ((InputPort1 & BUTTON_START) && !(InputPort1Prev & BUTTON_START)) {
                InitLevel();
                FamiToneSfxPlay(SFX_START);
            }
            break;
        case STATE_LEVEL:
            if ((InputPort1 & BUTTON_A) && !(InputPort1Prev & BUTTON_A)) {
                PlayerAction();
            }
            MovePlayer();
            break;
        case STATE_CREDITS:
            break;
        default:
            break;
    }
}


void UpdateCredits() {
    if(FrameCount == (FRAMES_PER_SEC / 10)) {
        for(i = 0; i < N_TEXT_SPRITES; ++i) {
            // scroll message offscreen if not last message
            // if it is last message, just scroll to top and freeze
            if(text_locs[i] >= CREDITS_MIN_Y &&
               (credits_msg < CREDITS_MSGS - 1 ||
                text_locs[N_TEXT_SPRITES - 1] >= CREDITS_MIN_Y + 1 + SPRITE_HEIGHT * CREDITS_MSG_LEN - CREDITS_MSG_LEN))
            {
                text_locs[i] -= 1;
            }

            if(text_locs[i] >= CREDITS_MIN_Y && text_locs[i] < CREDITS_MAX_Y + SPRITE_HEIGHT) {
                text_sprites[i].y = text_locs[i];
            } else {
                text_sprites[i].y = SPRITE_OFFSCREEN_Y;
            }
        }
        FrameCount = 0;

        // if a message scrolled offscreen, queue the next one
        // last message will never scroll offscreen
        if(text_sprites[0].y == SPRITE_OFFSCREEN_Y &&
           text_sprites[N_TEXT_SPRITES - 1].y == SPRITE_OFFSCREEN_Y)
        {
            ++credits_msg;
            InitCreditsSprites();
        }

        // when all scrolling is done, show the date
        if(credits_msg == CREDITS_MSGS - 1 &&
           text_locs[N_TEXT_SPRITES - 1] <= CREDITS_MIN_Y + 1 + SPRITE_HEIGHT * CREDITS_MSG_LEN - CREDITS_MSG_LEN)
        {
            ++credits_msg;

            DisablePPU();
            ResetScroll();

            ppu_addr = PPU_NAMETABLE_0 + (CREDITS_DATE_ROW * NUM_COLS) + CREDITS_DATE_COL;
            PPU_ADDRESS = (uint8_t)(ppu_addr >> 8);
            PPU_ADDRESS = (uint8_t)(ppu_addr);
            for (i = 0; i < CREDITS_DATE_LEN; ++i) {
                PPU_DATA = CHR_TO_IDX[CREDITS_DATE[i]];
            }

            WaitVBlank();
            EnablePPU();
        }
    }
}


void UpdatePlayerSprites() {
    for(i = 0; i < 4; ++i) {
        for(j = 0; j < 4; ++j) {
            player_sprites[i * 4 + j].x = player.x + j * SPRITE_WIDTH;
            player_sprites[i * 4 + j].y = player.y + i * SPRITE_HEIGHT;
            player_sprites[i * 4 + j].tile_index = player_sprite_data[i * 4 + j];
        }
    }
}

void UpdateFade() {
    if(FrameCount >= FRAMES_PER_SEC / 2) {
        FrameCount = 0;
        ++fade_count;
        if(fade_count >= 4) {
            ResetScroll();
            FadeStep();
        }
        if(fade_count == 10) {
            InitCredits();
        }
    }
}

void Update() {
    switch(state) {
    case STATE_CREDITS:
        UpdateCredits();
        break;
    case STATE_LEVEL:
        UpdatePlayerSprites();
        break;
    case STATE_FADE:
        UpdateFade();
        break;
    default:
        break;
    }
}


void main(void) {
    FamiToneInit();
    FamiToneSfxInit();
    InitTitle();

    // turn on rendering
    ResetScroll();
    EnablePPU();

    while (1) {
        WaitFrame();
        FamiToneUpdate();
        HandleInput();
        Update();
        ResetScroll();
    };
};

