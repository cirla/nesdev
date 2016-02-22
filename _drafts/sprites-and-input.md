---
title: "Sprites and Input"
layout: post
git_branch: sprites_input
---

{% assign branch_url = page.git_branch | prepend: "/tree/" | prepend: site.github_url %}

What good is a video game unless we can actually play it?
In this lesson, we're going to cover capturing input from standard controllers and acting on that input every frame to move a sprite around the screen.
Check out [the finished product]({{branch_url}}/sprites_input.nes) to see it in action.

![Screenshot]({{site.baseurl}}/images/sprites_input/screenshot.png)

Firstly, we're going to need to add some new sprites to our pattern table.
A great tool for editing NES tilesets is [YY-CHR](http://www.geocities.jp/yy_6502/yychr/index.html) which is a Windows program but runs decently in [wine](https://www.winehq.org/) on Linux and OS X.
It comes in two flavors, the [newer .NET version](http://www.geocities.jp/yy_6502/yychr/yychr_net.zip) and the [older C++ version](http://www.geocities.jp/yy_6502/yychr/yy-chr20120407_en.zip).
I find that the .NET version is generally your best bet, though copy/paste seems to work better in the C++ version when running in `wine`.

Let's add some background tiles to make a border around the screen and a tiny one-tile character (we'll get into using metasprites to make a multi-tile character in a later post).

![Pattern Table]({{site.baseurl}}/images/sprites_input/pattern_table.png)

## Something Familiar

Remember how to draw background tiles to the pattern table?
Let's do that again.
First, we'll create some handy aliases for the tile indices representing our background (and the sprite that we'll use later).

{% highlight c %}
#define BLANK_TILE    0x00
#define BORDER_TL     0x01
#define BORDER_TR     0x02
#define BORDER_BL     0x11
#define BORDER_BR     0x12
#define BORDER_T      0x04
#define BORDER_B      0x14
#define BORDER_L      0x03
#define BORDER_R      0x13
#define SPRITE_PLAYER 0x10
{% endhighlight %}

We'll also create a background palette (we'll only need one for this lesson) that will color our border tiles and a sprite palette (again, we'll just need one) to use for later.
After writing the palette data to the PPU, we'll set the `PPU_ADDRESS` to the beginning of the nametable and then loop through and fill in our border tiles:

{% highlight c %}
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
{% endhighlight %}

Since we're only using one background palette, and it's palette 0, we don't have to do anything with the attribute table.
Now let's move on to something new!

## That Shrewd and Knavish Sprite

Information about sprites on the NES is stored in a special 256-byte chunk of memory in the PPU called [OAM](http://wiki.nesdev.com/w/index.php/PPU_OAM), or Object Attribute Memory.
Each sprite is represented by four bytes, which means that we can display up to 64 sprites on the screen.
The PPU can only render eight sprites on the same scan line; those nearer the beginning of OAM taking precedence over those near the end.

We can write data to OAM very similarly to how we write to the rest of the PPU using the `OAM_ADDRESS` and `OAM_DATA` [registers](http://wiki.nesdev.com/w/index.php/PPU_registers), but there's actually a more efficient way to do it.
The NES actually has a [DMA](https://en.wikipedia.org/wiki/Direct_memory_access) (Direct Memory Access) mechanism to write an entire page of RAM (256 bytes) directly into the PPU's OAM much more quickly than we could looping through and writing one byte at a time into `OAM_DATA`.
If you remember from the introduction to vblank in the last post, we're limited to writing to PPU memory (including OAM) during the (very, very short) vblank period, so we want to save as many cycles as we can.

Firstly, we're going to set aside a page of RAM to be used as a buffer that we'll write to OAM every frame:

{% highlight text %}
MEMORY {
…
    OAM: start = $0200, size = $100;
…
}
…
SEGMENTS {
…
    OAM: load = OAM, type = bss, define = yes;
…
}
{% endhighlight %}

The `define = yes` for the OAM segment defines the linker symbol `__OAM_LOAD__` (it will be `0x0200` corresponding to the start address we set for the memory section).
We can now very easily modify our NMI handler to write the buffer to OAM every frame via DMI.
We do this by setting `OAM_ADDRESS` to `0` (start writing to the beginning of OAM) and then writing the high byte of the RAM address (`0x02` for `0x0200`) to `OAM_DMA`:

{% highlight asm %}
; start OAM DMA
lda #0
sta OAM_ADDRESS
lda #>(__OAM_LOAD__)
sta OAM_DMA
{% endhighlight %}

Now we can update our buffer anytime we want and any changes will be pushed to the PPU during the next vblank.

**TODO:** Sprite struct and initialization.

## Pulling the Strings

**TODO:** Input handling

## Next Time

In the next post, we'll look at more efficient ways to create and draw static (non-scrolling) backgrounds, using metasprites to represent multi-tile sprites, and animating our sprites.

