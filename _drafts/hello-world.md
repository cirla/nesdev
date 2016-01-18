---
title: Hello, World!
layout: post
git_branch: hello_world
---

{% assign branch_url = page.git_branch | prepend: "/tree/" | prepend: site.github_url %}

Our goal for this first lesson is to create the "Hello, World!" equivalent of an NES ROM.
To see it in action, load [the finished product]({{branch_url}}/hello_world.nes) in your NES emulator of choice.
You should see something like this in all its glory:

![Screenshot]({{site.baseurl}}/images/hello_world/screenshot.png)

It doesn't seem like much, but just getting to this point requires a decent amount of knowledge of both how the NES hardware and our toolchain works.

## The C Code

The best place to start is probably [hello_world.c]({{ "/hello_world.c" | prepend: branch_url }}), where the code that actually produces our "Hello, World!" screen lives.

The first thing you'll notice is the list of `#define`s all beginning with `PPU_`:

{% highlight c %}
#define PPU_CTRL    *((uint8_t*)0x2000)
#define PPU_MASK    *((uint8_t*)0x2001)
#define PPU_STATUS  *((uint8_t*)0x2002)
#define PPU_SCROLL  *((uint8_t*)0x2005)
#define PPU_ADDRESS *((uint8_t*)0x2006)
#define PPU_DATA    *((uint8_t*)0x2007)
{% endhighlight %}

These are aliases to specific memory addresses that are mapped to 8-bit registers in the [PPU](http://wiki.nesdev.com/w/index.php/PPU)&mdash;the Picture Processing Unit, which generates the video signal sent to the TV&mdash;thus exposing them to code executed on the CPU.
These [registers](http://wiki.nesdev.com/w/index.php/PPU_registers) allow us to communicate with the PPU and tell it
what we want to appear on the screen.

Next, we have a few definitions for [color values](http://wiki.nesdev.com/w/index.php/PPU_palettes#RGB_PPU_palettes) used to define palettes:

{% highlight c %}
#define COLOR_BLACK 0x0f
#define COLOR_WHITE 0x20
{% endhighlight %}

The PPU is capable of a limited set of pre-defined colors which can be combined in palettes and assigned to sprites.
Each color can be represented in a single byte, hence the term "8-bit graphics."

Next, we're defining a few variables, but there are a few things to explain.
First, the `#pragma bss-name` is a pragma provided by [cc65](http://cc65.github.io/cc65/), the C compiler we're using to target the [6502 CPU](http://wiki.nesdev.com/w/index.php/CPU) in the NES.
What [the pragma](http://cc65.github.io/doc/cc65.html#ss7.1) does is allow us to specify where in memory we want variables to be defined.
We'll get to where we define these memory areas in a bit, but first we'll talk about how memory is laid out on the NES.

The NES CPU has 2 KiB of internal RAM, split into eight 256-byte pages, with each address represented as a two-byte (16-bit) [word](https://en.wikipedia.org/wiki/Word_(computer_architecture)).
The first (or zero-th) page is called the zero page, as all of its addresses begin with the byte `0x00`.
The [6502 instruction set](https://en.wikibooks.org/wiki/6502_Assembly) has special instructions for dealing with zero page memory addressing that take into account the implicit zero byte and therefore run a little faster than the same instructions performed on memory in other pages.
Since we're writing code for a 30-year-old piece of hardware that runs at a blazing 1.79 MHz, we'll want to take advantage of the zero page for variables that are most often used in our game.
What's used more often in a C program than a loop counter?

{% highlight c %}
#pragma bss-name(push, "ZEROPAGE")
size_t i;
#pragma bss-name(pop)
{% endhighlight %}

Why, you may ask, are we defining this globally instead of using a local stack variable?
Loop counters will be often used and we want to take advantage of the extra speed in using zero page memory (our C stack is not located in the zero page).

Next up, we're going to define (in a memory section set up in other pages of RAM), the rest of the data we need for this game.

{% highlight c %}
#pragma bss-name(push, "RAM")
const char TEXT[] = "Hello, World!";

const uint8_t PALETTE[] = {
    COLOR_BLACK,                           // background color
    COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, // background palette 0
};
#pragma bss-name(pop)
{% endhighlight %}

First is the text that we want to display on the screen.
Simple enough.

Next, we're going to define a palette to be used to render the background.
Palette data has to be in a [specific format](http://wiki.nesdev.com/w/index.php/PPU_palettes#Memory_Map) expected by the PPU.
Aside from a universal background color (we're using black), we can define up to four different background palettes (we only need one), each with three colors (again, we only need one&mdash;white).
Each 16x16 pixel area of the background will be associated with a background palette and can make use of its three colors in addition to the background color.
As we're creating a very simple background and not using any sprites, we're only worried about a single background palette.

Now that we have all the data set up, let's move on to our `main()`:

{% highlight c %}
void main(void) {
{% endhighlight %}

One thing that may look out of place for C developers is the fact that `main` takes no arguments (we're running directly on the hardware; there's no OS/shell to pass in arguments) and returns no value (again, there's nowhere to which we `return`).

Thanks to some startup code we've written elsewhere (we'll get to it later), we can assume that memory, the PPU, etc. are in a known state by the time we get to `main`.
RAM is zeroed, all rendering, sound, and interrupts are disabled, and the PPU is ready to receive data.
The first thing we'll do is send our palette data over to the PPU:

{% highlight c %}
PPU_ADDRESS = 0x3f;
PPU_ADDRESS = 0x00;
for ( i = 0; i < sizeof(PALETTE); ++i ) {
    PPU_DATA = PALETTE[i];
}
{% endhighlight %}

To understand what's going on here, we have to know that neither the CPU nor the PPU have direct access to each other's memory.
When writing data to the PPU, we first have to tell it where in its memory (there are 2 KiB of VRAM in the PPU&mdash;though game cartridges could provide more&mdash;as well as a little memory for palettes and sprite data) we want to read or write data from.
This is done by writing first the upper followed by the lower byte of the target address to the `PPU_ADDRESS` register.
[Palette data](http://wiki.nesdev.com/w/index.php/PPU_palettes#Memory_Map) is stored to be in addresses `0x3f00` through `0x3f1f` in PPU RAM.
We write one byte at a time to the `PPU_DATA` register.
Each read or write to this register increments the `PPU_ADDRESS` by one (this can be configured by a flag in `PPU_CTRL` that we've set up in our initialization code).

Now that the palette information is copied over to the PPU, we'll move on to the background tiles that make up our "Hello, World!" greeting.

{% highlight c %}
PPU_ADDRESS = 0x21;
PPU_ADDRESS = 0xca;
for ( i = 0; i < sizeof(TEXT); ++i ) {
    PPU_DATA = TEXT[i];
}
{% endhighlight %}

Looking at the [PPU memory map](http://wiki.nesdev.com/w/index.php/PPU_memory_map), you'll see that the address we're writing to is located in the `0x2000`-`0x23FF` range designated for Nametable 0.
A [nametable](http://wiki.nesdev.com/w/index.php/PPU_nametables) is a 1 KiB chunk of memory that represents the background.
There are four such nametables addressable by the PPU and we can switch between them using a flag in `PPU_CTRL` (our startup code initialized this to Nametable 0).
You may have noticed that there are four 1 KiB nametables but only 2 KiB of VRAM; two of them are just mirrors of the other two (whether mirroring is horizontal or vertical can be configured, though it doesn't really matter until we get to scrolling backgrounds) unless additional VRAM is provided on the cartridge.

**TODO: image of nametables laid out in VRAM**

Each of the first 960 bytes in the nametable represent which 8px × 8px tile in the pattern table (we'll talk about these soon) to display in that 8px × 8px area of the screen.
The background is composed of 30 rows, each with 32 tiles.
The remaining 64 bytes in the nametable make up the [attribute table](http://wiki.nesdev.com/w/index.php/PPU_attribute_tables), which specifies which palettes are used for each 16px × 16px area of the screen.

**TODO: image of layout within nametable**

By writing to address `0x21ca`, we're writing into Nametable 0 with an offset of `0x1ca` (the 458th tile, i.e. row 14 column 10), which will center our "Hello, World!" text on the screen.
What we are actually writing to `PPU_DATA` here one byte at a time are offsets into the pattern table, which we've conveniently created to store the tiles making up our font in offets corresponding to the [ASCII value](https://en.wikipedia.org/wiki/ASCII#ASCII_printable_code_chart) they represent (e.g. the tile representing the letter `A` is stored at offset `0x41`).

**TODO: image of pattern table**

Notice that tile 0 is a blank tile, which makes up the majority of our background.

**TODO: PPU SCROLL**
{% highlight c %}
PPU_SCROLL = 0x00;
PPU_SCROLL = 0x00;
{% endhighlight %}

**TODO: PPU CTRL/MASK**
{% highlight c %}
PPU_CTRL = 0x80;
PPU_MASK = 0x1e;
{% endhighlight %}

Our work being done, we command the CPU to spin forever as we bask in the glory of our creation.

{% highlight c %}
while (1) {};
{% endhighlight %}

## Memory Layout

**TODO: memory layouts in loader config file**

## Initialization Code

**TODO: init code in reset.s**

