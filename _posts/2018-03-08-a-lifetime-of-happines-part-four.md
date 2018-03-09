---
title: "A Lifetime of HappiNES: Part 4"
layout: post
git_branch: wedding
---

{% assign branch_url = page.git_branch | prepend: "/tree/" | prepend: site.github_url %}

[Last time]({{ site.baseurl }}{% post_url 2018-02-08-a-lifetime-of-happines-part-three %}) we finished up our static level screen.
Let's add in our player next and let him stretch his legs a little.

## Enter Stage Left

Our player is going to be much larger than 8px × 8px, so we'll need to use multiple sprites.
For each direction our player can face, we'll need a different arrangement of 16 sprites (4×4) to fully render him.
Let's start by creating a mapping of tile indices that correspond with each of his possible states:

{% highlight c %}
const uint8_t GROOM_FRONT[4][4] = {
  {0x6D, 0x6E, 0x6F, 0x1F},
  {0x77, 0x78, 0x79, 0x1F},
  {0x81, 0x82, 0x83, 0x84},
  {0x8D, 0x8E, 0x8F, 0x90},
};

const uint8_t GROOM_BACK[4][4] = {
...
const uint8_t GROOM_LEFT[4][4] = {
...
const uint8_t GROOM_RIGHT[4][4] = {
...
{% endhighlight %}

We'll also reserve a chunk of OAM memory to hold all of the necessary sprite information.

{% highlight c %}

typedef struct sprite {
    uint8_t y;          // y pixel coordinate
    uint8_t tile_index; // index into pattern table
    uint8_t attributes; // attribute flags
    uint8_t x;          // x pixel coordinate
} sprite_t;

...

#pragma bss-name(push, "OAM")
sprite_t player_sprites[16]; // 4x4 grid
#pragma bss-name(pop)
{% endhighlight %}

We could just identify the player's overall position by any one of his sprites (top-left, for instance), but let's keep
track of state outside of OAM.

{% highlight c %}
typedef struct player {
    uint8_t x;
    uint8_t y;
} player_t;

player_t player;
{% endhighlight %}

We're finally ready to render him on the screen!

{% highlight c %}
#define PLAYER_START_X  20
#define PLAYER_START_Y 180

...

void InitLevel() {
    ...

    // initialize player
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player_sprite_data = GROOM_FRONT[0];

    for(i = 0; i < 4; ++i) {
        for(j = 0; j < 4; ++j) {
            player_sprites[i * 4 + j].x =
                player.x + j * SPRITE_WIDTH;
            player_sprites[i * 4 + j].y =
                player.y + i * SPRITE_HEIGHT;
            player_sprites[i * 4 + j].tile_index =
                player_sprite_data[i * 4 + j];
            // bottom half of groom (i < 2) uses different
            // palette than top half (i >= 2)
            player_sprites[i * 4 + j].attributes =
                i < 2 ? 0x01 : 0x02;
        }
    }

    ...
}
{% endhighlight %}

There are a few important things to point out here.
Firstly, we're using [sprite attributes](https://wiki.nesdev.com/w/index.php/PPU_OAM#Byte_2) to set two different palettes for our groom's top half (first two rows) and bottom half (last two rows).
This is akin to how we set background attribute data for him in the static credits screen.

Secondly, it appears that we're using multiplication in our code, and the NES CPU does not have any multiplication instructions!
We're not using a fancy cartridge mapper like the [MMC5 which came with its own 8-bit multiplier](https://wiki.nesdev.com/w/index.php/MMC5#Multiplier_.28.245205.2C_read.2Fwrite.29), and `cc65` multiplication subroutines seem to always mess up my ROM, so I'm being very careful to ensure that all runtime multiplication that we do is by a power of two so that it can be accomplished by left shift instructions.

Lastly, we declared `GROOM_FRONT` as a 2D array, but here we're getting a pointer to the first element (`GROOM_FRONT[0]`) and then working with that as though it's a 1D array.
For some reason, accessing it in the conventional C way to access 2D arrays results in generated assembly that does not work as desired.

Keep these last two points in mind as we work with 2D arrays and looping over them in nested loops.

## It's Alive!

Now that he's on the screen, let's move him around with controller input.
First, we'll add some code to update the player's state.

{% highlight c %}
void MovePlayer() {
    if (InputPort1 & BUTTON_UP) {
        --player.y;
        player_sprite_data = GROOM_BACK[0];
    }

    if (InputPort1 & BUTTON_DOWN) {
        ++player.y;
        player_sprite_data = GROOM_FRONT[0];
    }

    if (InputPort1 & BUTTON_LEFT) {
        --player.x;
        player_sprite_data = GROOM_LEFT[0];
    }

    if (InputPort1 & BUTTON_RIGHT) {
        ++player.x;
        player_sprite_data = GROOM_RIGHT[0];
    }
}

void HandleInput() {
    switch(state) {
    ...
    case STATE_LEVEL:
        MovePlayer();
        break;
    ...
    }
}
{% endhighlight %}

Next, we'll want to update the sprites based on that state every frame.
This is the same code we used to initialize the player's sprites, but we no longer need to worry about setting the palette attributes.

{% highlight c %}
void UpdatePlayerSprites() {
    for(i = 0; i < 4; ++i) {
        for(j = 0; j < 4; ++j) {
            player_sprites[i * 4 + j].x =
                player.x + j * SPRITE_WIDTH;
            player_sprites[i * 4 + j].y =
                player.y + i * SPRITE_HEIGHT;
            player_sprites[i * 4 + j].tile_index =
                player_sprite_data[i * 4 + j];
        }
    }
};

void Update() {
    switch(state) {
    ...
    case STATE_LEVEL:
        UpdatePlayerSprites();
        break;
    ...
}
{% endhighlight %}

Our player can now move freely about the screen!
A little too freely, though.

## Setting Boundaries

Keeping our player inside the confines of the border we drew is as easy as enforcing min and max positions.
We'll make use of our `MIN_X`, `MIN_Y`, `MAX_X`, `MAX_Y` constants that provide pixel boundaries for the screen as well as `SPRITE_HEIGHT` and `SPRITE_WIDTH`.

The top five rows of our screen are off limits, so we'll make sure that `player.y` (the top of our player) is below (greater than) `MIN_Y + SPRITE_HEIGHT * 5`.
We can't multiply by five at runtime, so let's translate that to multiplying by the closest power of two (four) and then adding one more, or `MIN_Y + SPRITE_HEIGHT * 4 + SPRITE_HEIGHT`.

Only the bottom row of the screen is off limits, but we have to account for the fact that `player.y` is the top of our player and our player is four sprites tall, for a total of five `SPRITE_HEIGHT`s, which yields a very similar looking restriction.
We can extrapolate this to all of the boundaries:

{% highlight c %}
void MovePlayer() {
    if (InputPort1 & BUTTON_UP) {
        if (player.y > MIN_Y + SPRITE_HEIGHT * 4 + SPRITE_HEIGHT) {
            --player.y;
            player_sprite_data = GROOM_BACK[0];
        }
    }

    if (InputPort1 & BUTTON_DOWN) {
        if (player.y < MAX_Y - (4 * SPRITE_HEIGHT) - SPRITE_HEIGHT) {
            ++player.y;
            player_sprite_data = GROOM_FRONT[0];
        }
    }

    if (InputPort1 & BUTTON_LEFT) {
        if (player.x > MIN_X + SPRITE_WIDTH) {
            --player.x;
            player_sprite_data = GROOM_LEFT[0];
        }
    }

    if (InputPort1 & BUTTON_RIGHT) {
        if (player.x < MAX_X - (4 * SPRITE_WIDTH) - SPRITE_WIDTH) {
            ++player.x;
            player_sprite_data = GROOM_RIGHT[0];
        }
    }
}
{% endhighlight %}

Now our player can only move within the borders we drew in our background, but he can still stomp all over his bride-to-be!

## Collisions

Let's add a simple [<abbr title="axis-aligned bounding box">AABB</abbr> collision detection routine](https://developer.mozilla.org/en-US/docs/Games/Techniques/2D_collision_detection) to be able to check if our player is touching anything (for now just the bride).

{% highlight c %}
#define BRIDE_X    208
#define BRIDE_Y     48

// height and width of person (bride and groom)
#define PERSON_DIM  32

bool DetectCollision() {
    return (
        player.x < BRIDE_X + PERSON_DIM &&
        player.x + PERSON_DIM > BRIDE_X &&
        player.y < BRIDE_Y + PERSON_DIM &&
        PERSON_DIM + player.y > BRIDE_Y
    );
}
{% endhighlight %}

And we'll just backtrack state if moving our player would result in an overlap:

{% highlight c %}
void MovePlayer() {
    if (InputPort1 & BUTTON_UP) {
        if (player.y > MIN_Y + SPRITE_HEIGHT * 4 + SPRITE_HEIGHT) {
            --player.y;
            if(DetectCollision()) { ++player.y; }
            player_sprite_data = GROOM_BACK[0];
        }
    }
    ...
}
{% endhighlight %}

Our bride-to-be is now made of solid matter!

## The Ring

We can add the ring onto the level very similarly to how we initialized the player, but our ring won't be moving (except to predetermined locations), so we don't need to track its state the same way.

{% highlight c %}
#define RING_START_X 120
#define RING_START_Y 112

...

#pragma bss-name(push, "OAM")
sprite_t player_sprites[16]; // 4x4 grid
sprite_t ring_sprites[4];    // 2x2 grid
#pragma bss-name(pop)

...

void InitLevel() {
    ...

    for(i = 0; i < 2; ++i) {
        for(j = 0; j < 2; ++j) {
            ring_sprites[i * 2 + j].x =
                RING_START_X + j * SPRITE_WIDTH;
            ring_sprites[i * 2 + j].y =
                RING_START_Y + i * SPRITE_HEIGHT;
            ring_sprites[i * 2 + j].tile_index =
                RING_SPRITES[0][i * 2 + j];
        }
    }
    ...
}
{% endhighlight %}

We'll also want to make the ring solid, so let's add it to our collision detection routine.
Let's change the return type and add a little more info about which thing our player is colliding with to make things
easier later when we want to interact with them.

{% highlight c %}
#define RING_DIM 16

...

typedef enum collision {
    COLLISION_NONE = 0, // false
    COLLISION_RING,
    COLLISION_BRIDE
} collision_t;

collision_t DetectCollision() {
    if (player.x < BRIDE_X + PERSON_DIM &&
        player.x + PERSON_DIM > BRIDE_X &&
        player.y < BRIDE_Y + PERSON_DIM &&
        PERSON_DIM + player.y > BRIDE_Y) {
        return COLLISION_BRIDE;
    }

    if (ring_sprites[0].x == RING_START_X &&
        player.x < RING_START_X + RING_DIM &&
        player.x + PERSON_DIM > RING_START_X &&
        player.y < RING_START_Y + RING_DIM &&
        PERSON_DIM + player.y > RING_START_Y) {
        return COLLISION_RING;
    }

    return COLLISION_NONE;
}
{% endhighlight %}

Conveniently, when used in an `if` condition, `COLLISION_NONE` is `0`, which will evaluate as `false`, and the others values will all evaluate as `true`, so we can leave our `MovePlayer` code as is.

Now that the ring has a corporeal form, our player should be able to pick it up.

# Taking Action

We want our player to be able to push the A button and pick up the ring if both:
1. The ring has not yet been picked up.
2. He is facing it and directly adjacent to it in the direction he is facing.

The first one is easy; we can just check if the ring's position is still the same (`ring_sprites[0].x == RING_START_X`).
The second one is a little trickier.
First, we'll determine which direction he is facing.
Then, we'll pretend he moved one pixel in that direction and we'll check if that causes him to collide with the ring.
Finally, we'll undo that one pixel move.

If we determine that he satisfies those conditions, we'll play a little sound effect and move the ring up into the inventory box.

{% highlight c %}
#define RING_INV_X 236
#define RING_INV_Y  11

...

void PlayerAction() {
    if(ring_sprites[0].x == RING_START_X) {
        i = 0;
        if(player_sprite_data == GROOM_FRONT[0]) {
            ++player.y;
            if(DetectCollision() == COLLISION_RING) { i = 1; }
            --player.y;
        }
        else if(player_sprite_data == GROOM_BACK[0]) {
            --player.y;
            if(DetectCollision() == COLLISION_RING) { i = 1; }
            ++player.y;
        }
        else if(player_sprite_data == GROOM_LEFT[0]) {
            --player.x;
            if(DetectCollision() == COLLISION_RING) { i = 1; }
            ++player.x;
        }
        else if(player_sprite_data == GROOM_RIGHT[0]) {
            ++player.x;
            if(DetectCollision() == COLLISION_RING) { i = 1; }
            --player.x;
        }

        if(i) {
            FamiToneSfxPlay(SFX_RING);
            for(i = 0; i < 2; ++i) {
                for(j = 0; j < 2; ++j) {
                    ring_sprites[i * 2 + j].x =
                        RING_INV_X + j * SPRITE_WIDTH;
                    ring_sprites[i * 2 + j].y =
                        RING_INV_Y + i * SPRITE_HEIGHT;
                }
            }
        }
    }
}

...

void HandleInput() {
    switch(state) {
    case STATE_LEVEL:
        ...
        if ( (InputPort1 & BUTTON_A) &&
            !(InputPort1Prev & BUTTON_A)) {
            PlayerAction();
        }
        MovePlayer();
        break;
        ...
    }
}
{% endhighlight %}

With the ring in his grasp, our player's final task is to propose.

## A Modest Proposal

We've done all the hard work already; this is going to be very similar to interacting with the ring.
If our player is both facing and directly adjacent to the bride-to-be in the direction he is facing, **and** he has the
ring in his inventory, we'll play the sound effect again, move the ring to the bride, and transition to the credit screen.

To make things even easier, we can ignore the possibility of our player being above or to the right of the bride-to-be since she's right up against the border in the top-right corner.

{% highlight c %}
void PlayerAction() {
    if(ring_sprites[0].x == RING_START_X) {
        ...
    } else if(ring_sprites[0].x == RING_INV_X) {
        i = 0;
        if(player_sprite_data == GROOM_BACK[0]) {
            --player.y;
            if(DetectCollision() == COLLISION_BRIDE) { i = 1; }
            ++player.y;
        }
        else if(player_sprite_data == GROOM_RIGHT[0]) {
            ++player.x;
            if(DetectCollision() == COLLISION_BRIDE) { i = 1; }
            --player.x;
        }

         if(i) {
            FamiToneSfxPlay(SFX_RING);
            for(i = 0; i < 2; ++i) {
                for(j = 0; j < 2; ++j) {
                    ring_sprites[i * 2 + j].x =
                        BRIDE_X + j * SPRITE_WIDTH + 5;
                    ring_sprites[i * 2 + j].y =
                        BRIDE_Y + i * SPRITE_HEIGHT + 14;
                }
            }
            InitCredits();
        }
    }
}
{% endhighlight %}

This sudden transition feels jarring and anti-climactic.
We only see the bride with her ring for a fraction of a second!
That's all for now, but next time we'll add a slow fade-to-black before we roll credits.

