---
title: "Color in Motion"
layout: post
git_branch: color_in_motion
---

{% assign branch_url = page.git_branch | prepend: "/tree/" | prepend: site.github_url %}

Our goal for this lesson is to expand our knowledge of the PPU to spice up our "Hello, World!" ROM into something a bit more exciting.
To see it in action, load [the finished product]({{branch_url}}/color_in_motion.nes) in your NES emulator of choice.
You should see something like this:

![Demo]({{site.baseurl}}/images/color_in_motion/demo.gif)

## Keeping Organized

The first thing you might notice about [our code]({{branch_url}}) is that we have a few new files.
To make our code more readable, there are now a bunch of useful `#define`s for memory addresses and other constants in [nes.h]({{branch_url}}/nes.h).
This will prevent us from having to do the math in our heads&mdash;when setting `PPU_CTRL`, for example, we can just or (`|`) together the flags we want to set.
A few of these constants (e.g. framerate) will differ between [NTSC](https://en.wikipedia.org/wiki/NTSC) and [PAL](https://en.wikipedia.org/wiki/PAL) PPUs, so we'll have to define either `TV_NTSC` or `TV_PAL`, e.g.:

{% highlight c %}
#define TV_NTSC 1
#include "nes.h"
{% endhighlight %}

We've also created [data.h]({{branch_url}}/data.h) to define things like palette data to keep our [main source file]({{branch_url}}/color_in_motion.c) cleaner and [reset.h]({{branch_url}}/reset.h) to include some new symbols we're exporting from [reset.s]({{branch_url}}/reset.s).

## Counting Frames

Taking a closer look at [reset.h]({{branch_url}}/reset.h) we see it's just two short lines of code:

{% highlight c %}
extern uint8_t FrameCount;
#pragma zpsym("FrameCount");
{% endhighlight %}

The first is telling the compiler that we're going to refer to a symbol named `FrameCount` that is defined externally (e.g. not in the C file from which this header is included) as an unsigned byte.
The second line is a [pragma provided by cc65](http://cc65.github.io/doc/cc65.html#ss7.16) to tell the compiler that an externally defined symbol is a zero page symbol.

Let's go take a look in [reset.s]({{branch_url}}/reset.s) where `FrameCount` is defined:

{% highlight asm %}
.exportzp _FrameCount

…

.segment "ZEROPAGE"

_FrameCount: .res 1
{% endhighlight %}

You'll notice that we've prepended an underscore to the variable name as `cc65` will prepend an underscore to all C symbols in assembly, e.g. `jmp _main`.
The `.exportzp` line tells the assembler to export the symbol `_FrameCount` as zero page and the other lines are actually defining the symbol as a single byte in the `ZEROPAGE` segment.

### The Non-Maskable Interrupt

We can figure out pretty easily by its name what this `_FrameCount` variable is all about, but how are we actually keeping count of frames?
There are a couple ways to detect a frame change, but the simplest and most straightforward is the [non-maskable interrupt](http://wiki.nesdev.com/w/index.php/NMI), or NMI.
We can instruct the PPU (via a flag in `PPU_CTRL`) to generate an NMI after it is done generating the video signal for every frame and then write some code in our NMI handler to do something useful:

{% highlight asm %}
nmi:
    inc _FrameCount
    rti
{% endhighlight %}

For now we're just going to increment the frame count and return from the interrupt handler.
Now we can detect when the frame changes and how many frames have elapsed!

### Attributes

### Vertical Blanking

