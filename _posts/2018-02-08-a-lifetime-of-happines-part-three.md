---
title: "A Lifetime of HappiNES: Part 3"
layout: post
git_branch: wedding
---

{% assign branch_url = page.git_branch | prepend: "/tree/" | prepend: site.github_url %}

[Last time]({{ site.baseurl }}{% post_url 2018-02-03-a-lifetime-of-happines-part-two %}) we added some music and sound effects to our title screen.
Let's add in some placeholders for the level and credits screens so we can put the rest of our music where it belongs.

## Transitions and States

Until now we've just had a single state for our game: the title screen.
Our game will have very little state to track, so let's go with a simple enumeration:

{% highlight c %}
typedef enum state {
    STATE_TITLE,
    STATE_LEVEL,
    STATE_CREDITS
} state_t;

state_t state;
{% endhighlight %}

For each state, we'll need an `Init` function to draw its appropriate background and play its song, e.g.:

{% highlight c %}
void InitLevel() {
    // disable PPU so we can write to PPU memory without flicker
    DisablePPU();

    state = STATE_LEVEL;

    // write palettes
    ppu_addr = PPU_PALETTE;
    ppu_data = PAL_LEVEL;
    ppu_data_size = sizeof(PAL_LEVEL);
    WritePPU();

    // draw background
    bg = BG_CREDITS;
    DrawBackground();

    FamiToneMusicPlay(SONG_LEVEL);

    // re-enable PPU at next VBlank
    WaitVBlank();
    EnablePPU();
}
{% endhighlight %}


Let's whip up some very basic placeholder backgrounds in NESst using the same CHR tiles as our title screen:

![Background Placeholders]({{site.baseurl}}/images/wedding/placeholders.png)

Once we've exported and created our header files for these backgrounds, all that's left is handling the transition when a player presses Start.

{% highlight c %}
void HandleInput() {
    switch(state) {
    case STATE_TITLE:
        if ( (InputPort1 & BUTTON_START) &&
            !(InputPort1Prev & BUTTON_START)) {
            InitLevel();
            FamiToneSfxPlay(SFX_START);
        }
        break;
    case STATE_LEVEL:
        if ( (InputPort1 & BUTTON_START) &&
            !(InputPort1Prev & BUTTON_START)) {
            InitCredits();
        }
        break;
    case STATE_CREDITS:
    default:
        break;
    }
}
{% endhighlight %}

Now that we have our screens with music, let's make them more interesting.

## Finishing Backgrounds

The pattern table that contains the tiles for our title screen is starting to get pretty full:

![Title CHR]({{site.baseurl}}/images/wedding/chr_title.png)

Luckily, we have a second pattern table that's completely empty thus far.
We have to consider, though, that we'll be sharing it between both the level and credits screens.
The easiest way to do this is to create one image that will contain all the tiles we need for both screens (both backgrounds and sprites) and run it through `makechr` all together.

For both screens, we'll need some characters from the [same font we used on the title screen](https://fontstruct.com/fontstructions/show/1205992/8_bit_6x6_nostalgia), so we'll just throw in the whole alphabet and some common punctuation.
For the level screen, we'll want a border, [some sprites for Elizabeth and Philip](https://opengameart.org/content/nes-style-rpg-characters), and [a ring](https://alexchanyan.itch.io/16x16-rpg-item-pack).
For the credits screen, we can re-use the same border and sprites, but let's add [a silhouette of the crown]({{site.baseurl}}/images/wedding/crown_clipart.png) for flair.
After resizing and manual touch-ups to get everything within NES PPU restrictions, we have something like this:

![Level and Credits Mockup]({{site.baseurl}}/images/wedding/bg_level_credits.png)

Now we can run this through `makechr` and add the CHR data onto the second half of our existing CHR file:

{% highlight c %}
makechr -e error.png bg_level_credits.png
# back up old data
mv sprites.chr sprites.chr.bak
# get the first 4k bytes from our existing CHR file (pattern table 0)
head -c 4096 sprites.chr.bak > sprites.chr
# append the first 4k bytes from the CHR data makechr just generated
# to the end (pattern table 1)
head -c 4096 chr.dat >> sprites.chr
{% endhighlight %}

`makechr` is a great tool, but it's not always perfect, so some tiles that are intended to be used in the same palette may be generated with the colors swapped around.
With a little bit of manual touchup in NESst or YY-CHR, we should have something like this:

![Level and Credits CHR]({{site.baseurl}}/images/wedding/chr_level_credits.png)

Unlike before where we just used `makechr`'s nametable and attribute output to create our nametable, attribute, and palette data, we have to open up NESst and manually create our backgrounds by placing tiles where we want them and tweaking the palettes.
For the crown I did cheat and use `makechr`'s output as a starting point because I did not want to re-assemble the crown puzzle.

![Level Background]({{site.baseurl}}/images/wedding/bg_level.png)
![Credits Background]({{site.baseurl}}/images/wedding/bg_credits.png)

Now just export the palette and RLE'd nametable data as before and our placeholders have been replaced by colorful backgrounds, except the screen appears to be filled with garbage:

![Level Background with Wrong Pattern Table]({{site.baseurl}}/images/wedding/bg_level_wrong_pattern_table.png)

This is because the PPU still thinks we're still using pattern table 0 and is using tiles from the title screen.
Since we're disabling and re-enabling the PPU for each state transition, this is a good time to tell the PPU which pattern table we want to use.

{% highlight c %}
uint8_t pattern_table; // 0 or 1
...
void EnablePPU() {
    i = PPUCTRL_NAMETABLE_0 | // use nametable 0
        PPUCTRL_NMI_ON      ; // enable NMIs

    if(pattern_table == 0) {
        i |= (PPUCTRL_BPATTERN_0 | PPUCTRL_SPATTERN_0);
    } else {
        i |= (PPUCTRL_BPATTERN_1 | PPUCTRL_SPATTERN_1);
    }

    PPU_CTRL = i;
    ...
}
...
void InitLevel() {
    DisablePPU();
    ...
    pattern_table = 1;
    ...
    EnablePPU();
}
{% endhighlight %}

And now our backgrounds should render correctly on all screens!
That's all for now, but next time we'll finish up the level screen with sprites, collisions, and inventory.

