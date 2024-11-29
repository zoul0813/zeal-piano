#include <stdint.h>
#include <zvb_gfx.h>
#include <zvb_sound.h>

#ifndef PIANO_H
#define PIANO_H

#define ACTION_NONE         0
#define ACTION_PAUSE        1
#define ACTION_CONTINUE     1
#define ACTION_QUIT         10

#define SCREEN_WIDTH        320
#define SCREEN_HEIGHT       240
#define WIDTH               20
#define HEIGHT              15

#define LEVEL_LAYER         0
#define UI_LAYER            1

#define TILEMAP_OFFSET      0x00U
#define TILE_EMPTY          0x00U

#define TILE_OCTAVE         8
#define OCTAVE_X            2
#define OCTAVE_Y            13

#define TILE_WAVEFORM       11
#define WAVEFORM_X          4
#define WAVEFORM_Y          13
#define WAVEFORM_MIN        0
#define WAVEFORM_MAX        2

#define TILE_RECORD         16
#define TILE_PLAY           19
#define STATE_INDEX         0
#define STATE_X             32
#define STATE_Y             32

#define TILE_NUMBER         48
#define VOICES_X            6
#define VOICES_Y            13

#define TILE_PROGRESS       21

#define VOLUME_MIN          0
#define VOLUME_MAX          4

#define ONSCREEN_KEY_WHITE  2
#define ONSCREEN_KEY_BLACK  3
#define ONSCREEN_KEY_PRESS  4

typedef uint8_t error;

void init(void);
void deinit(void);
uint8_t input(void);
void load_tilemap(void);

void update(void);
void draw(void);

void set_waveform(uint8_t w);
void set_octave(uint8_t o);
void set_volume(int8_t v);

#endif