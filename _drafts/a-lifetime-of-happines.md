---
title: A Lifetime of HappiNES
layout: post
git_branch: wedding
---

{% assign branch_url = page.git_branch | prepend: "/tree/" | prepend: site.github_url %}

It's been a while since the [last post]({{ site.baseurl }}{% post_url 2016-02-22-sprites-and-input %}), but this is going to be a special one.

A friend of mine recently got hitched to a pretty rad lady, and I didn't want him unprepared for the quest ahead.
He just happens to be a retro gaming console collector and enthusiast (not to mention one of the top-ranked [Duck Hunt](https://en.wikipedia.org/wiki/Duck_Hunt) players in the world), so I figured I'd try making them a custom NES game, cartridge and all.
This post is going to cover that process all the way through, including all of the manual and messy bits.

Partly to maintain anonymity, but mostly to keep their gift a special one-of-a-kind collector's item, for this post we'll instead be making a gift to send back in time to [Princess Elizabeth's wedding](https://en.wikipedia.org/wiki/Wedding_of_Princess_Elizabeth_and_Philip_Mountbatten,_Duke_of_Edinburgh).

## The Idea

Unfortunately this idea didn't strike me until a few weeks before the wedding, so I had to keep the project scope at a realistically achievable level.
I planned for three main parts:

 - A title screen inspired by one of their engagement photos
 - A simple level where the groom would stash a ring in his inventory and use it to propose
 - A credit screen inspired by their wedding invitation where we could wish them the best

I chose the [NES-NROM-128](https://wiki.nesdev.com/w/index.php/NROM) as my target cartridge board for maximum simplicity.
16 KiB of PRG ROM would be more than enough and I'd just have to make do with two 4KiB banks of CHR ROM.

## Development

I borrowed a lot of code from my earlier posts to get this project jumpstarted, so I won't be covering things like [common assembly code](https://github.com/cirla/nesdev/blob/wedding/asm/reset.s) or [linker configuration](https://github.com/cirla/nesdev/blob/wedding/nes-nrom-128.cfg) unless there's something that I haven't covered before.
I'm going to primarily focus on [the game logic](https://github.com/cirla/nesdev/blob/wedding/src/main.c) and asset creation.

To start, let's create a skeleton for our `main()` function that will set everything up to render the title screen then
kick off rendering and jump into the main game loop:

{% highlight c %}
void main(void) {
    InitTitle();

    // turn on rendering
    ResetScroll();
    EnablePPU();

    while (1) {
        WaitFrame();
        HandleInput();
        Update();
        ResetScroll();
    };
};
{% endhighlight %}

### Title Screen

I was inspired by the bride and groom's awesome engagement photos and wanted them featured prominently on the title
screen.
For our game, let's use this great photograph of a young Philip and Elizabeth as our inspiration:

![Philip and Elizabeth]({{site.baseurl}}/images/wedding/original.jpg)
*Source: [Old Magazine Articles](http://www.oldmagazinearticles.com/magazine-articles/european_royalty_articles/elizabeth_ii_articles)*

I'm not much of an artist, so I'm going to let the computer do as much work as possible and then try to make things look
nice at the end.
Let's open up a [Jupyter notebook](http://jupyter.org/) and put [Pillow](https://pillow.readthedocs.io) to work.
First, I want to get this down to a reasonable size.
Something that will fit within our 256x240 resolution with room to spare for some text.

{% highlight python %}
from PIL import Image, ImageFilter

img = Image.open('./original.jpg').convert('RGB')
img.thumbnail((150, 150)) # resize and maintain aspect ratio
img
{% endhighlight %}

![Philip and Elizabeth - Resized]({{site.baseurl}}/images/wedding/resized.png)

Next, let's [quantize](https://en.wikipedia.org/wiki/Color_quantization) the image to only use colors from the [NES PPU palette](https://en.wikipedia.org/wiki/List_of_video_game_console_palettes#NES).
First, we'll scrape the RGB data using [Beautiful Soup](https://www.crummy.com/software/BeautifulSoup/) and store it in a [pandas](https://pandas.pydata.org/) `DataFrame`.

{% highlight python %}
import ast
import re

from bs4 import BeautifulSoup
import pandas as pd
import requests

response = requests.get('https://en.wikipedia.org/wiki/List_of_video_game_console_palettes#NES')
soup = BeautifulSoup(response.text, 'lxml')

palette_colors = [
    # extract RGB tuples from CSS in style attribute and eval as python tuples
    ast.literal_eval(re.search('(\(.*\))', el['style']).group(1))
    for el in soup.find(id='NES') \
                  .find_next(text='Hex Value') \
                  .find_previous('table') \
                  .find_all('td', style=re.compile('background:rgb'))]

palette_colors = pd.DataFrame(palette_colors, columns=['R', 'G', 'B'])
palette_colors['Hex'] = palette_colors.index.map(lambda i: f'${i:02x}')
palette_colors.head(5)
{% endhighlight %}

| | R | G | B | Hex |
|-|---|---|---|-----|
| 0 | 124 | 124 | 124 | $00 |
| 1 | 0 | 0 | 252 | $01 |
| 2 | 0 | 0 | 188 | $02 |
| 3 | 68 | 40 | 188 | $03 |
| 4 | 148 | 0 | 132 | $04 |

Next, we'll use this to create a palette and force the image to use only these colors.

{% highlight python %}
import itertools

# flatten RGB values into tuple i.e. (R1, G1, B1, R2, G2, B2, ...)
pil_palette = tuple(itertools.chain.from_iterable(
    palette_colors[['R', 'G', 'B']].itertuples(index=False)))

# pad tuple to 256 colors
pil_palette += (0, 0, 0) * (256 - len(palette_colors))

# create palette image with NES color palette
palette_img = Image.new('P', (1, 1))
palette_img.putpalette(pil_palette)

# this will dither
# quantized = img.quantize(palette=palette_img)

# use `ImagingCore` directly to avoid dithering
q = img.im.convert('P', 0, palette_img.im)
quantized = img._new(q)

quantized
{% endhighlight %}

![Philip and Elizabeth - Quantized]({{site.baseurl}}/images/wedding/quantized.png)

Finally, we'll use a [median filter](https://en.wikipedia.org/wiki/Median_filter) to smooth out the image into large contiguous chunks of colors.
This will make it easier to see the more important features of the image because we'll have to work within very tight palette restrictions to render this on the NES.

{% highlight python %}
quantized.convert('RGB').filter(ImageFilter.MedianFilter(3))
{% endhighlight %}

![Philip and Elizabeth - Filtered]({{site.baseurl}}/images/wedding/filtered.png)

This looks terrible, but it's a great starting canvas.
Now it's time to open up your pixel editor of choice and [draw the rest of the owl](http://knowyourmeme.com/photos/572078-how-to-draw-an-owl).
I tried to go for the fewest colors that still preserved the general look of the image and ended up with this:

![Philip and Elizabeth - Final]({{site.baseurl}}/images/wedding/final.png)

To make the full title screen, we paste that image into a new 256x240 canvas, find a nice [6x6 pixel font](https://fontstruct.com/fontstructions/show/1205992/8_bit_6x6_nostalgia) to fit inside the 8x8 tiles, and add some text.
It's important that we place the letters along an 8x8 grid so that we only need one tile in CHR memory per letter.

![Title Screen]({{site.baseurl}}/images/wedding/bg_title.png)

{% highlight sh %}
makechr -e error.png bg_title.png
{% endhighlight %}

{% highlight sh %}
cp nametable.dat bg_title.nam
cat attributes.dat >> bg_title.name
head -c 16 palette.dat > bg_title.pal
{% endhighlight %}

{% highlight c %}
uint8_t const * bg;            // background data

void DrawBackground() {
    PPU_ADDRESS = (uint8_t)(PPU_NAMETABLE_0 >> 8);
    PPU_ADDRESS = (uint8_t)(PPU_NAMETABLE_0);
    UnRLE(bg);
}
{% endhighlight %}

{% highlight c %}
void InitTitle() {
    DisablePPU();

    // write palettes
    ppu_addr = PPU_PALETTE;
    ppu_data = PAL_TITLE;
    ppu_data_size = sizeof(PAL_TITLE);
    WritePPU();

    // draw background
    bg = BG_TITLE;
    DrawBackground();

    WaitVBlank();
    EnablePPU();
}
{% endhighlight %}

## Level

## Credits

## Music and Sound Effects

## The Cartridge

