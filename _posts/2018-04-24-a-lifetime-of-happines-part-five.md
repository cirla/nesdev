---
title: "A Lifetime of HappiNES: Fade &amp; Credits"
layout: post
git_branch: wedding
---

{% assign branch_url = page.git_branch | prepend: "/tree/" | prepend: site.github_url %}

[Last time]({{ site.baseurl }}{% post_url 2018-03-08-a-lifetime-of-happines-part-four %}) we added the actual gameplay to our game, but it still needs some polish.

## Fade to Black

We want a nice dramatic fade to black before we roll credits.
Instead of diving right into `InitCredits();` when we hand over the ring, let's add a new `STATE_FADE` and `InitFade();` instead.

We still want to play the credits song immediately, but we won't switch the background just yet.
To achieve our fade, we're going to make a copy of the current palette (background and sprites) and periodically darken it until every color is black.
We'll keep track of time via our `FrameCount` variable that's updated in the NMI handler every frame and our region-specific PPU framerate (`FRAMES_PER_SEC`).

{% highlight c %}
uint8_t fade_palette[sizeof(PAL_LEVEL)];
uint8_t fade_count;

void InitFade() {
    state = STATE_FADE;
    FamiToneMusicPlay(SONG_CREDITS);

    FrameCount = 0;
    fade_count = 0;

    // copy level palette
    for ( i = 0; i < sizeof(PAL_LEVEL); ++i ) {
        fade_palette[i] = PAL_LEVEL[i];
    }
}
{% endhighlight %}

For our `UpdateFade` function, we'll keep track of how much time has elapsed and take action accordingly:

1. Wait two seconds and let the player enjoy victory
2. Fade to black every half second over the next two seconds
3. Wait another second and roll credits

{% highlight c %}
void UpdateFade() {
    // take a step every half second
    if(FrameCount >= FRAMES_PER_SEC / 2) {
        FrameCount = 0;
        ++fade_count;

        // after the first two seconds, fade
        if(fade_count >= 4) {
            ResetScroll();
            FadeStep();
        }

        // after five seconds, roll credits
        if(fade_count == 10) {
            InitCredits();
        }
    }
}
{% endhighlight %}

For our `FadeStep` method, we want to take our copy of the level palette, make every color a darker shade, then write the altered copy back to `PPU_PALETTE` in PPU RAM.

If we look at [the NES color palette](https://en.wikipedia.org/wiki/List_of_video_game_console_palettes#NES), we can see that the colors range from `0x00` to `0x3F` and—conveniently—the colors in each column (i.e. with the same first hex digit) are roughly shades of the same color from dark to light.

To accomplish our fade, let's just subtract `0x10` from every color in the palette until they've reached black:

{% highlight c %}
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
{% endhighlight %}

And now we have a nice dramatic transition to our credits!
For good measure, let's write the bride's response to the background just before fading.

If you recall from [a previous post]({{ site.baseurl }}{% post_url 2018-02-08-a-lifetime-of-happines-part-three %}), we have the entire alphabet (and a good bit of punctuation) in the pattern table that we're using for the level and credits screens.
I'm too lazy to rearrange the CHR ROM so that the index of each character's sprite corresponds with its ASCII code, so let's create a mapping table so we can use human-readable strings in our code:

{% highlight c %}
// map ASCII character to sprite index
const uint8_t CHR_TO_IDX[96] = {
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
     31,  30,   0,   0,   0,   0,   0,  28,
      0,   0,   0,   0,  27,   0,  26,   0,
    158, 159, 160, 161, 162, 163, 164, 164,
    166, 167,   0,   0,   0,   0,   0,  29,
      0,   0,   1,   2,   3,   4,   5,   6,
      7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,
     23,  24,  25,   0,   0,   0,   0,   0,
};
{% endhighlight %}

And now something like `CHR_TO_INDEX['B']` will yield index `1`, the index of the **B** sprite.
We can now define a message and write it out to a location in the background:

{% highlight c %}
#define FADE_MSG_ROW 12
#define FADE_MSG_COL 14
#define FADE_MSG_LEN  4
const char FADE_MSG[] = "YES!";

void InitFade() {
    ...
    DisablePPU();
    ResetScroll();

    ppu_addr =  PPU_NAMETABLE_0 +
               (FADE_MSG_ROW * NUM_COLS) +
                FADE_MSG_COL;
    PPU_ADDRESS = (uint8_t)(ppu_addr >> 8);
    PPU_ADDRESS = (uint8_t)(ppu_addr);
    for (i = 0; i < FADE_MSG_LEN; ++i) {
        PPU_DATA = CHR_TO_IDX[FADE_MSG[i]];
    }

    WaitVBlank();
    EnablePPU();
}
{% endhighlight %}

We're also going to make good use of this `CHR_TO_INDEX` map to render the credits.

## Roll Credits

While setting background tiles was good enough for writing static text, for our credits we want precision at the pixel level and we want our text to move, so we're going to need to use sprites.

We want to display one message at a time, moving to the next when it's finished scrolling.
We'll cap each message to four rows of text at eight characters per row (notice the choice of powers of two so we can do run-time multiplication using shifts) to fit inside our box.


{% highlight c %}
// # of messages
#define CREDITS_MSGS 3

// # of lines per message
#define CREDITS_MSG_LEN 4

// # of lines total
#define CREDITS_LEN (CREDITS_MSGS * CREDITS_MSG_LEN)

// # of characters per line
#define CREDITS_WIDTH 8

// sprites needed to render one message at a time
#define N_TEXT_SPRITES (CREDITS_MSG_LEN * CREDITS_WIDTH)

// message data (+1 for NUL terminator)
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
{% endhighlight %}

We needed to use `CREDITS_WIDTH + 1` instead of just `CREDITS_WIDTH` because the C compiler inserts a `NULL` (`\0`) byte at the end of string literals.
It's a few wasted bytes, but worth it so that we can write our data in human-readable text.

We'll also need to set some boundaries where the sprites are allowed to exist:

{% highlight c %}
#define CREDITS_MIN_X  96
#define CREDITS_MIN_Y 100
#define CREDITS_MAX_Y 200
{% endhighlight %}

And reserve some OAM RAM for our text sprites:

{% highlight c %}
#pragma bss-name(push, "OAM")
...
sprite_t text_sprites[N_TEXT_SPRITES];
#pragma bss-name(pop)
{% endhighlight %}

We'll also want to keep track of which credits message to display and we're going to buffer changes to the sprites' y coordinates so that we can handle collisions with the boundaries in a clean way.

{% highlight c %}
uint8_t credits_msg; // index into CREDITS_TEXT
uint8_t text_locs[N_TEXT_SPRITES];
{% endhighlight %}

We're now ready to initialize the credits:

{% highlight c %}
void InitCredits() {
    ...

    state = STATE_CREDITS;
    pattern_table = 1;
    credits_msg = 0;

    ...

    InitCreditsSprites();
    FrameCount = 0;

    WaitVBlank();
    EnablePPU();
}

void InitCreditsSprites() {
    for(i = 0; i < CREDITS_MSG_LEN; ++i) {
        for(j = 0; j < CREDITS_WIDTH; ++j) {
            // save some redundant typing
            #define OFFSET (i * CREDITS_WIDTH + j)

            // init buffered y coords
            text_locs[OFFSET] = CREDITS_MAX_Y +
                                SPRITE_HEIGHT +
                                SPRITE_HEIGHT * i;

            // init sprite x coords and tiles
            text_sprites[OFFSET].x = CREDITS_MIN_X +
                                     SPRITE_WIDTH * j;

            text_sprites[OFFSET].tile_index =
                CHR_TO_IDX[
                    CREDITS_TEXT[0][
                        // beginning of message
                        (credits_msg *
                         CREDITS_WIDTH *
                         CREDITS_MSG_LEN) +
                        // null terminators
                        (credits_msg * CREDITS_MSG_LEN) + i +
                        // row offset within message
                        (i * CREDITS_WIDTH) +
                        // offset in current line
                        j
                    ]
                ];
            #undef OFFSET
        }
    }
}
{% endhighlight %}

The null terminators throw a wrench in things, but we were able to work around them.
Now we can write our `UpdateCredits` to scroll our credits within our bounding box from bottom to top, moving sprites off screen as they reach the top.
You can play with the speed, but I found 10 pixels/second a good balance.

{% highlight c %}
void UpdateCredits() {
    if(FrameCount == (FRAMES_PER_SEC / 10)) {
        FrameCount = 0;

        for(i = 0; i < N_TEXT_SPRITES; ++i) {
            // scroll message up one pixel.
            // if it is last message, don't let it
            // scroll beyond the top
            if(text_locs[i] >= CREDITS_MIN_Y &&
               (credits_msg < CREDITS_MSGS - 1 ||
                text_locs[N_TEXT_SPRITES - 1] >= CREDITS_MIN_Y + 1 +
                    SPRITE_HEIGHT * CREDITS_MSG_LEN -
                    CREDITS_MSG_LEN))
            {
                text_locs[i] -= 1;
            }

            // if buffered y is in bounds, set to sprite
            // otherwise, move sprite off screen
            if(text_locs[i] >= CREDITS_MIN_Y &&
               text_locs[i] < CREDITS_MAX_Y + SPRITE_HEIGHT) {
                text_sprites[i].y = text_locs[i];
            } else {
                text_sprites[i].y = SPRITE_OFFSCREEN_Y;
            }
        }

        // if a message scrolled offscreen, queue the next one
        // last message will never scroll offscreen
        if(text_sprites[0].y == SPRITE_OFFSCREEN_Y &&
           text_sprites[N_TEXT_SPRITES - 1].y == SPRITE_OFFSCREEN_Y)
        {
            ++credits_msg;
            InitCreditsSprites();
        }
    }
}
{% endhighlight %}

And last but not least, we want to write the date to the screen after the last message has finished scrolling.
Since it's not going to move, we can just draw directly to the background:

{% highlight c %}
#define CREDITS_DATE_ROW 27
#define CREDITS_DATE_COL 10
#define CREDITS_DATE_LEN 12
const char CREDITS_DATE[] = "NOV. 20 1947";

void UpdateCredits() {
    if(FrameCount == (FRAMES_PER_SEC / 10)) {
        ...

        // when all scrolling is done, show the date
        if(credits_msg == CREDITS_MSGS - 1 &&
           text_locs[N_TEXT_SPRITES - 1] <= CREDITS_MIN_Y + 1 +
               SPRITE_HEIGHT * CREDITS_MSG_LEN -
               CREDITS_MSG_LEN)
        {
            ++credits_msg;

            DisablePPU();
            ResetScroll();

            ppu_addr =  PPU_NAMETABLE_0 +
                       (CREDITS_DATE_ROW * NUM_COLS) +
                        CREDITS_DATE_COL;
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
{% endhighlight %}

Our ROM is now complete!
Next time we'll go over getting it onto a physical cartridge.

