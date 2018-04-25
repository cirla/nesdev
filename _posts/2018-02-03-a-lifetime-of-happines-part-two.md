---
title: "A Lifetime of HappiNES: Music &amp; SFX"
layout: post
git_branch: wedding
---

{% assign branch_url = page.git_branch | prepend: "/tree/" | prepend: site.github_url %}

[Last time]({{ site.baseurl }}{% post_url 2018-01-18-a-lifetime-of-happines-part-one %}) we left off with just a nifty title screen, but it seems kind of dull.
Let's add some music!

## Music is the Strongest Form of Magic

I will readily admit that I have very close to zero talent in the realm of music, so I knew I was going to need some help on this front.
I made a post on [/r/gameDevClassifieds](https://www.reddit.com/r/gameDevClassifieds/) tagged `Musician Wanted` laying out my requirements:

  - I needed a few songs; one each for the title, level, and credits screens.
  - The music had to be composed in [FamiTracker](http://famitracker.com/) so that I could easily export it to a convenient data format.
  - Ideally, the music was also composed with limitations described in [Doug Fraker's blog post regarding NES music](https://nesdoug.com/2015/12/02/15-adding-music/) so that I could use [Shiru](https://shiru.untergrund.net/code.shtml)'s lightweight [FamiTone2](https://shiru.untergrund.net/files/src/famitone2.zip) library for playback.

Despite my unusual requirements, I got quite a few replies!
I think the fact that this was `[PAID]` work was a big part of that, but I was pleasantly surprised with the <acronym title="gameDevClassifieds">GDC</acronym> community.
I ended up going with [Mitch Foster](http://www.mitchfostermusic.com/) for the job, and I highly recommend his work and professionalism.
In addition to freelancing, he's also a sound engineer at [Mega Cat Studios](https://megacatstudios.com/) which has some great NES development resources on [their blog](https://megacatstudios.com/blogs/press).

Mitch provided me with three songs (including the [Bridal Chorus](https://en.wikipedia.org/wiki/Bridal_Chorus) for the credits) in a FamiTracker .ftm file and a handful of sound effects in another.
The next step was to open each file in FamiTracker and export to text (_File_→_Export Text..._) and <acronym title="Nintendo Sound File">NSF</acronym> (_File_→_Create NSF..._), respectively.
Then, we could use the handy tools provided with Shiru's FamiTone2 library to convert these to `ca65` assembly.
I'm developing on a Macbook, so I have to run these Windows executables via [Wine](https://www.winehq.org/).

{% highlight shell %}
wine famitone2/tools/text2data.exe -ca65 music.txt
wine famitone2/tools/nsf2data.exe -ca65 sfx.nsf
{% endhighlight %}

And now we should have a [music.s]({{branch_url}}/data/music/music.s) and [sfx.s]({{branch_url}}/data/music/sfx.s), but
what do we do with them?

The FamiTone2 library comes with library code for playing these data files back, but we have to modify it a bit to be able to use it in our C code.
You can diff our modified [famitone2.inc]({{branch_url}}/lib/famitone2.inc) and [famitone2.s]({{branch_url}}/lib/famitone2.s) with those provided by Shiru to see the full changes, but we'll focus on the important parts.

There are five functions that we want to expose:

1. `FamiToneInit` to reset the APU and initialize the library
2. `FamiToneSfxInit` to initialize SFX playback
3. `FamiToneMusicPlay` to set the currently playing song
4. `FamiToneUpdate` to update FamiTone state every frame
3. `FamiToneSfxPlay` to play a sound effect

For these, let's add new symbols with the same names (except prefixed with an underscore so that they'll be callable from C) just above these assembly functions.
In some cases, they take arguments passed through the A, X, and Y registers.
We'll be hardcoding some of these values and using the [`fastcall` calling convention](https://github.com/cc65/wiki/wiki/Parameter-passing-and-calling-conventions#The_fastcall_calling_convention) to make sure calling our C functions with arguments puts the right values in the right registers.

{% highlight shell %}
diff -w -U 1 tools/famitone2/famitone2.s lib/famitone2.s
{% endhighlight %}
{% highlight diff %}
--- tools/famitone2/famitone2.s
+++ lib/famitone2.s
...
+_FamiToneInit:
+    ldx #<MUSIC_DATA
+    ldy #>MUSIC_DATA
 FamiToneInit:
...
+_FamiToneMusicPlay:
 FamiToneMusicPlay:
...
 +_FamiToneUpdate:
 FamiToneUpdate:
...
+_FamiToneSfxInit:
+    ldx #<SFX_DATA
+    ldy #>SFX_DATA
 FamiToneSfxInit:
 ...
+_FamiToneSfxPlay:
+    ldx #0
+
 FamiToneSfxPlay:
{% endhighlight %}

Notice that we're hardcoding the addresses of `MUSIC_DATA` and `SFX_DATA`.
We're including the data files we generated before inside [reset.s]({{ branch_url }}/asm/reset.s).

{% highlight asm %}
.segment "RODATA"

MUSIC_DATA:
.include "music.s"

SFX_DATA:
.include "sfx.s"
{% endhighlight %}

Now, let's make sure to export these symbols and put them in the right memory segment.

{% highlight shell %}
diff -w -U 0 tools/famitone2/famitone2.inc lib/famitone2.inc
{% endhighlight %}
{% highlight diff %}
--- tools/famitone2/famitone2.inc
+++ lib/famitone2.inc
@@ -2,0 +3,5 @@
+.export _FamiToneInit, _FamiToneSfxInit, _FamiToneMusicPlay, _FamiToneSfxPlay, _FamiToneUpdate
+
+.segment "CODE"
+.include "famitone2.s"
+
@@ -6 +11 @@
-.segment "FAMITONE"
+.segment "BSS"
@@ -9,2 +13,0 @@
-.segment "CODE"
-.include "famitone2.s"
{% endhighlight %}

Now, we can include the library from our reset.s, but first we'll need to set a few options so that it will compile with the features we need.

{% highlight asm %}
.define FT_PAL_SUPPORT  0 ; I don't need to support PAL
.define FT_NTSC_SUPPORT 1 ; Include NTSC support
.define FT_PITCH_FIX    0 ; Needed when supporting both NTSC and PAL
.define FT_THREAD       1 ; Safe calling from NMI handler
.define FT_DPCM_ENABLE  0 ; We don't need DMC support
.define FT_SFX_ENABLE   1 ; Include SFX support
FT_SFX_STREAMS = 1        ; Support playing one sound effect at a time
FT_DPCM_OFF    = $c000    ; Unused default; Not using DMC
.include "famitone2.inc"
{% endhighlight %}

And finally, we'll need a C header to define these functions for the C compiler.

{% highlight c %}
void __fastcall__ FamiToneInit(void);
void __fastcall__ FamiToneSfxInit(void);
void __fastcall__ FamiToneMusicPlay(uint8_t song);
void __fastcall__ FamiToneSfxPlay(uint8_t effect);
void FamiToneUpdate(void);
{% endhighlight %}

We're now ready to go back and add the music to our title screen:

{% highlight c %}
#define SONG_TITLE 0 // 1st song in music.ftm
...
void InitTitle() {
    // write palettes
    ppu_addr = PPU_PALETTE;
    ppu_data = PAL_TITLE;
    ppu_data_size = sizeof(PAL_TITLE);
    WritePPU();

    // draw background
    bg = BG_TITLE;
    DrawBackground();

    FamiToneMusicPlay(SONG_TITLE);
}
...
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
{% endhighlight %}

We now have an exciting theme to listen to while admiring our title screen!
The screen very clearly instructs us to "Press Start", so let's play a sound effect every time we do.

{% highlight c %}
#define SFX_START 3 // 4th effect in sfx.ftm
...
void HandleInput() {
    if ( (InputPort1 & BUTTON_START) &&
        !(InputPort1Prev & BUTTON_START)) {
        FamiToneSfxPlay(SFX_START);
    }
}
{% endhighlight %}

Our game is really starting to come together!
That's all for now, but [next time]({{ site.baseurl }}{% post_url 2018-02-08-a-lifetime-of-happines-part-three %}) we're going to add our level and credits screens into the mix and handle transitions between them.

