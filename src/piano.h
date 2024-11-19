#include <stdint.h>
#include <zvb_gfx.h>

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
#define EMPTY_TILE          0x00U


#define TILE_WAVEFORM     11
#define WAVEFORM_X        4
#define WAVEFORM_Y        13
#define TILE_OCTAVE       8
#define OCTAVE_X          2
#define OCTAVE_Y          13

typedef uint8_t error;

void init(void);
void deinit(void);
uint8_t input(void);
void load_tilemap(void);

void update(void);
void draw(void);

#endif