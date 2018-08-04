---
title: "A Lifetime of HappiNES: The End"
layout: post
git_branch: wedding
---

{% assign branch_url = page.git_branch | prepend: "/tree/" | prepend: site.github_url %}

[Last time]({{ site.baseurl }}{% post_url 2018-04-24-a-lifetime-of-happines-part-five %}) we finished our game!
In this final post, we'll go over how to get it onto an NES cartridge with a personalized label.

## Materials

Since our ROM targeted the standard [NROM](https://wiki.nesdev.com/w/index.php/NROM), we can pick up an NES-NROM-256 board pretty cheaply from [Infinite NES Lives](http://www.infiniteneslives.com/nessupplies.php#NROM).
If you don't have an old NES cartridge lying around to salvage a case from, they also sell plastic cases and dust sleeves.

We'll also need a programmer to write the ROM data to the chip. I used the [Kazzo cartridge "INL Retro" Dumper-Programmer](http://www.infiniteneslives.com/kazzo.php) from Infinite NES Lives.

## Programming the ROM

First, we need to break the ROM file down into its constituent parts.
We'll discard the 16 byte iNES header, then split the remainder into the PRG ROM and CHR ROM.
Since we targeted the NES-NROM-128 with 16 KiB of PRG ROM but our chip is an NES-NROM-256 with 32 KiB of PRG ROM, we'll have to pad the file.
I just concatenated it to itself because I'm lazy.

{% highlight sh %}
# strip 16 byte iNES header
tail -c +17 wedding.nes > wedding.bin
# first 16 KiB is PRG ROM
head -c 16384 wedding.rom > prg.bin
# double to 32 KiB
cat prg.bin | tee >> prg.bin
# remaining 8 KiB is CHR ROM
tail -c +16385 wedding.bin > chr.bin
{% endhighlight %}

Next, I loaded these files on a USB stick and booted up into Windows to be able to use the Kazzo programmer.
Following the [README](https://www.dropbox.com/s/jg5j8s949o7lqox/readme.txt?dl=0), I was able to install [the drivers](https://www.dropbox.com/s/f15d778hoacz0sp/kazzo.zip?dl=1) in Windows 10 without too much hassle.

I had to disable driver signature enforcement by running `bcdedit.exe /set nointegritychecks on` at a `cmd.exe` prompt (Run as Administrator) and restarting.
After installing the drivers, I made sure to run `bcdedit.exe /set nointegritychecks off` to re-enable driver signature enforcement on future boots.

After successfully installing the drivers, I made sure to erase the chips on the NROM board (this board has both PRG and CHR ROM, so `ERASE_PRGCHR.bin` can erase both in one step).

Finally, I flashed the PRG and CHR ROM with my prg.bin and chr.bin files and popped the cartridge in my NES to test that it worked.

## Label

I found [this awesome .psd template for NES cartidges](https://drive.google.com/file/d/0ByHdtRdE3t7_U2xDZGQxX1BYWk0/view) from [John Riggs](http://gamester81.com/author/johnblueriggs/).
He's even got [a video tutorial](http://gamester81.com/how-to-make-your-own-nes-labels-download/) on how to create your own.
You can print it out on adhesive paper, though I had great results with regular printer paper and Mod Podge.

## The End

This is in all likelihood the final post in this series.
Thank you for reading and enjoy making your own NES games!
