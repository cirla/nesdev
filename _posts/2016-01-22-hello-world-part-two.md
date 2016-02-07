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
The flag `--add-source` includes our original C source as comments in the resulting assembly code (hello_world.s) so we can poke around if we're curious.

Next, we invoke [ca65](http://cc65.github.io/doc/ca65.html), a 6502 assembler, to assemble the output from `cc65` and our [NES startup code]({{ branch_url }}/reset.s) into [object files](https://en.wikipedia.org/wiki/Object_file) (6502 CPU machine code, though not yet executable).

Finally, we're invoking [ld65](http://cc65.github.io/doc/ld65.html), a linker, which combines our object files with its implementation of C standard library for the NES (notice the `nes.lib` in the command) into an executable NES ROM.
The `-o hello_world.nes` argument tells the linker that we'd like our executable to be named "hello_world.nes", while the `-C hello_world.cfg` tells the linker to use a configuration file that defines how memory is laid out, amongst other things. We now have an executable NES ROM!

### The Linker Configuration

We've created a [configuration file]({{ branch_url }}/hello_world.cfg) for `ld65` to do three things:

1. Define memory areas that represent ranges of addresses.
2. Define segments and lay out in which areas they belong.
3. Define symbols needed by our startup code.

#### Memory Areas

Defining the memory areas is as simple as defining a start address and a size and assigning it to a name:

{% highlight text %}
ZP: start = $00, size = $100;
RAM: start = $0300, size = $500;
{% endhighlight %}

The first two areas we create&mdash;`ZP` and `RAM`&mdash;will define sections of the NES CPU RAM, while the rest will define sections of the cartridge ROM chips.
We're defining `ZP` as an explicit area that represents the zero page we talked about in the last post.
It starts at memory address `0x0000` and is 256 (`0x100`) bytes in size.
We're going to skip over the next two pages for now (we'll use one for the C stack and make specific use of the other in later tutorials) and define the remaining five pages (`0x500` or 1280 bytes) as general-purpose RAM.

{% highlight text %}
HEADER: start = $0, size = $10, file = %O, fill = yes;
{% endhighlight %}

We're going to set aside sixteen (`0x10`) bytes at the beginning of our ROM file to hold the [iNES header](http://wiki.nesdev.com/w/index.php/INES) expected by NES emulators at the beginning of `.nes` files.
Later in this tutorial we'll see where that is populated.
The `file = %0` parameter tells the linker that the header will end up in the file passed in via the `-o` option, and `fill = yes` says that if we don't use the entire area, fill the remainder of the space with zero bytes.

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

You'll notice that we don't make the `PRG` area take up the full 16 KiB (`0x4000` bytes), but  six bytes short of that at `0x3ffa`.
We're explicitly setting aside six bytes of space for three 16-bit addresses that the CPU expects in a fixed space at the end of the cartridge space (i.e. `0xfffa`-`0xffff`).
These addresses, which we define in the `VECTORS` area, point to interrupt vectors&mdash;code that is run when interrupts are triggered&mdash;for three different types of interrupts expected by the CPU.
Two of these interrupts we don't care about just yet, but the other is the `reset` vector, which will point to our startup code.

{% highlight text %}
CHR: start = $0000, size = $2000, file = %O, fill = yes;
{% endhighlight %}

Finally, we create an area for our CHR ROM, which will contain our pattern tables.
The CHR ROM will be mapped to PPU memory addresses `0x0000`-`0x1fff`, though the cartridge may contain much more than 8 KiB (`0x2000` bytes) of CHR ROM (and/or CHR RAM) which is mapped to these addresses by the mapper.
Our simple NROM mapper just expects 8 KiB of CHR ROM which will map directly to the 8 KiB of PPU address space for pattern tables.

#### Segments

Now that our memory areas are set up, we can define segments and assign them to those memory areas.
If multiple segments belong in the same memory area, they are added in the order in which they appear here.
Segments are written to the output file in the order in which they appear as well, so we'll write our header first:

{% highlight text %}
HEADER: load = HEADER, type = ro;
{% endhighlight %}

Here we're creating a segment named `HEADER`, telling the linker that it belongs in the `HEADER` memory area, and marking it as read-only (`ro`) initialized data.
After the iNES header, emulators expect to find one or more 16 KiB PRG ROM banks, so we'll create a few segments that belong in the `PRG` memory area (as well as the `VECTORS` memory area, which is part of PRG ROM):

{% highlight text %}
STARTUP: load = PRG,            type = ro,  define = yes;
CODE:    load = PRG,            type = ro,  define = yes;
RODATA:  load = PRG,            type = ro,  define = yes;
DATA:    load = PRG, run = RAM, type = rw,  define = yes;
VECTORS: load = VECTORS,        type = ro;
{% endhighlight %}

The segments `CODE`, `RODATA`, and `DATA` are used by the C compiler to store read-only code, read-only data, and read/write data, respectively.
We're defining the `STARTUP` segment is used for storing read-only code that we've written in [reset.s]({{ branch_url }}/reset.s) to initialize the NES, and the `VECTORS` segment will store pointers to our interrupt handlers.

Because the `DATA` section is read/write (`rw`), and our ROM is read-only, anything in `DATA` will have to be loaded into `RAM`.
We use the `run = RAM` attribute to tell the linker that, even though `DATA` will reside in the `PRG` memory area, references to anything in `DATA` should use addresses from the `RAM` area.

After our PRG ROM banks, emulators will expect to find CHR ROM banks:

{% highlight text %}
CHARS: load = CHR, type = ro;
{% endhighlight %}

The `CHARS` segment is the last that will end up in our output file, though we still need to define a few more segments:

{% highlight text %}
ZEROPAGE: load = ZP,  type = zp;
BSS:      load = RAM, type = bss, define = yes;
{% endhighlight %}

We saw the `ZEROPAGE` segment in use in the last post when using `#pragma bss-name` in our C code to define zero page variables.
The `BSS` segment is used by the C compiler for uninitialized read/write data.
Since both of these have uninitialized types (`zp` and `bss`), they are not written to the output file.

#### Symbols

All that's left for us to do is tell the linker to export a couple of symbols we'll need to set up the C stack:

{% highlight text %}
__STACK_START__: type = weak, value = $0100;
__STACK_SIZE__:  type = weak, value = $100;
{% endhighlight %}

We're going to have the stack take up the second page of RAM (i.e. the 256 bytes starting at address `0x0100`).

## Startup Code

The last piece of the puzzle is our [scary assembly file]({{ branch_url }}/reset.s) that contains the NES startup code.
We won't explore this in great depth, but the file is liberally commented for the curious.

After importing a few symbols such as our C `main` function (notice the `_` prepended by the C compiler) which we'll need to call into, we define a few aliases for memory-mapped registers and plop our iNES header into the `HEADER` segment.

The `start` label is where we'll point our `reset` interrupt handler, and it contains a bunch of assembly code to get the NES into a known state (e.g. all interrupts disabled, rendering off, sound off, RAM zeroed, all sprites off-screen, PPU stabilized).
You'll see at the very end of this label we make use of our `__STACK_START__` and `__STACK_SIZE__` symbols to set up the C stack pointer before calling into our C `main` with `jmp _main`.

The remainder of the `STARTUP` section is defining the other two interrupt handlers which for now do nothing but `rti`&mdash;*return from interrupt*.

Finally, we set the addresses of our interrupt handlers in the `VECTORS` segment and include our pattern tables (via `.incbin`, which tells the assembler to load the "sprites.chr" file as-is and not to assemble it) into the `CHARS` segment.

That's it! We've covered everything that goes into making a simple NES ROM from scratch.

## Next Time

In the next post, [Color in Motion]({{ site.baseurl }}{% post_url 2016-02-06-color-in-motion %}), we'll create a more interesting game loop and explore using palettes to add color to our creation.

