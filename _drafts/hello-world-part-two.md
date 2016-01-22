---
title: "&ldquo;Hello, World!&rdquo; Part Two"
layout: post
git_branch: hello_world
---

{% assign branch_url = page.git_branch | prepend: "/tree/" | prepend: site.github_url %}

In [the first post]({{ site.baseurl }}{% post_url 2016-01-18-hello-world-part-one %}), we took a look at the C code that defines what our game does (which, for now, is just writing "Hello, World!" on the screen).
This time, we're going to explore [the toolchain](http://cc65.github.io/cc65/) that turns our code into a runnable ROM and the startup code that initializes the NES into a known state.

## Building the ROM

Taking a look at [our Makefile]({{ branch_url }}/Makefile), we see that we're running a few different commands to build our ROM:

{% highlight sh %}
cc65 -Oi hello_world.c --add-source
ca65 hello_world.s
ca65 reset.s
ld65 -C hello_world.cfg -o hello_world.nes reset.o hello_world.o nes.lib
{% endhighlight %}

The first command invokes [cc65](http://cc65.github.io/doc/cc65.html), a C compiler, to compile our C code into 6502 assembly.
The flag `-Oi` tells the compiler to optimize the generated code and to inline more aggressively, as we want to avoid making slow function calls.
The flag `--add-source` includes our original C source as comments in the resulting assembly code so we can poke around if we're curious.

Next, we invoke [ca65](http://cc65.github.io/doc/ca65.html), a 6502 assembler, to assemble the output from `cc65` and our [NES startup code]({{ branch_url }}/reset.s) into [object files](https://en.wikipedia.org/wiki/Object_file) (6502 CPU machine code, though not yet executable).

Finally, we're invoking [ld65](http://cc65.github.io/doc/ld65.html), a linker, which combines our object files with its implementation of C standard library for the NES (notice the `nes.lib` in the command) into an executable NES ROM.
The `-o hello_world.nes` argument tells the linker that we'd like our executable to be named "hello_world.nes", while the `-C hello_world.cfg` tells the linker to use a configuration file that defines how memory is laid out, amongst other things. We now have an executable NES ROM!

### The Linker Configuration

We've created a [configuration file]({{ branch_url }}/hello_world.cfg) for `ld65` to do three things:

1. Define memory sections that represent ranges of addresses.
2. Define memory segments and lay out in which sections they belong.
3. Define symbols needed by our startup code.

#### Memory Sections

Defining the memory sections is as simple as defining a start address and a size and assigning it to a name:

{% highlight text %}
ZP: start = $00, size = $100;
RAM: start = $0300, size = $500;
{% endhighlight %}

The first two sections we create&mdash;`ZP` and `RAM`&mdash;will define sections of the NES CPU RAM, while the rest will define sections of the cartridge ROM chips.
We're defining `ZP` as an explicit section that represents the zero page we talked about in the last post.
It starts at memory address `0x0000` and is 256 (`0x100`) bytes in size.
We're going to skip over the next two pages for now (we'll make specific use of them in later tutorials) and define the remaining five pages (`0x500` or 1280 bytes) as general-purpose RAM.

{% highlight text %}
HEADER: start = $0, size = $10, file = %O, fill = yes;
{% endhighlight %}

We're going to set aside sixteen (`0x10`) bytes at the beginning of our ROM file to hold the [iNES header](http://wiki.nesdev.com/w/index.php/INES) expected by NES emulators at the beginning of `.nes` files.
Later in this tutorial we'll see where that is populated.
The `file = %0` parameter tells the linker that the header will end up in the file passed in via the `-o` option, and `fill = yes` says that if we don't use the entire section, fill the remainder of the space with zeroes.

{% highlight text %}
PRG: start = $8000, size = $3ffa, file = %O, fill = yes;
{% endhighlight %}

The [NES CPU expects](http://wiki.nesdev.com/w/index.php/CPU_memory_map) the PRG ROM in the cartridge to map to the addresses `0x8000` through `0xffff`.
This is 32 KiB of address space, though cartridges may have as little as 16 KiB of PRG ROM as much as 1 MiB or more!
To be able to make use of more than 32 KiB of PRG ROM, cartidges have a [mapper](http://wiki.nesdev.com/w/index.php/Mapper) that can control which chunks of PRG ROM are mapped to which CPU addresses.
For this extremely simple game, we're going to use the simplest mapper: [NROM](http://wiki.nesdev.com/w/index.php/NROM) (more specifically the NES-NROM-128).
This mapper will map a single 16 KiB bank of PRG ROM to CPU addresses `0x8000`-`0xbfff`, which will also be mirrored in `0xc000`-`0xffff`.

{% highlight text %}
VECTORS: start = $fffa, size = $6, file = %O, fill = yes;
{% endhighlight %}

You'll notice that we don't make the `PRG` section take up the full 16 KiB (`0x4000` bytes), but  six bytes short of that at `0x3ffa`.
We're explicitly setting aside six bytes of space for three 16-bit addresses that the CPU expects in a fixed space at the end of the cartridge space (i.e. `0xfffa`-`0xffff`).
These addresses, which we define in the `VECTORS` section, point to interrupt vectors&mdash;code that is run when interrupts are triggered&mdash;for three different types of interrupts expected by the CPU.
Two of these interrupts we don't care about just yet, but the other is the `reset` vector, which will point to our startup code.

{% highlight text %}
CHR: start = $0000, size = $2000, file = %O, fill = yes;
{% endhighlight %}

Finally, we create a section for our CHR ROM, which will contain our pattern tables.
The CHR ROM will be mapped to PPU memory addresses `0x0000`-`0x1fff`, though the cartridge may contain much more than 8 KiB (`0x2000` bytes) of CHR ROM (and/or CHR RAM) which is mapped to these addresses by the mapper.
Our simple NROM mapper just expects 8 KiB of CHR ROM which will map directly to the 8 KiB of PPU address space for pattern tables.

#### Segments

#### Symbols

## Startup Code

## Next Time

