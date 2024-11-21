#include "piano.h"

#include <stdio.h>
#include <zgdk.h>
#include <zos_errors.h>
#include <zos_keyboard.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>
#include <zvb_gfx.h>
#include <zvb_hardware.h>

#include "assets.h"
#include "notes.h"
#include "track.h"

gfx_context vctx;
uint16_t frames = 0;
uint8_t controller_mode = 1;

uint16_t waveform = 1;
uint8_t octave = 1;

gfx_sprite sprite_record;
uint8_t sprite_record_frame = 0;

int main(void) {
    init();

    load_tilemap();

    while (true) {
        sound_loop();
        uint8_t action = input();
        switch (action) {
            case ACTION_QUIT:
                goto quit_game;
        }

        // frames++;
        // if (frames > 15) {
        //     frames = 0;
        // }

        if(_track.state != TRACK_NONE) {
            frames++;
        }

        update();
        draw();
    }

quit_game:
    deinit();

    return 0;
}

void init(void) {
    zos_err_t err;
    err = keyboard_init();
    if (err != ERR_SUCCESS) {
        printf("Failed to init keyboard: %d\n", err);
        exit(1);
    }
    err = keyboard_flush();
    if (err != ERR_SUCCESS) {
        printf("Failed to flush keyboard: %d\n", err);
        exit(1);
    }
    printf("keyboard initialized\n");

    // err = controller_init();
    // if (err != ERR_SUCCESS) {
    //     printf("Failed to init controller: %d", err);
    // }
    // err = controller_flush();
    // if (err != ERR_SUCCESS) {
    //     printf("Failed to flush controller: %d", err);
    // }
    // // verify the controller is actually connected
    // uint16_t test = controller_read();
    // // if unconnected, we'll get back 0xFFFF (all buttons pressed)
    // if (test & 0xFFFF) {
    //     controller_mode = 0;
    // }

    /* Disable the screen to prevent artifacts from showing */
    gfx_enable_screen(0);

    err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    if (err) exit(1);

    err = load_palette(&vctx);
    if (err) exit(1);

    gfx_tileset_options options = {
        .compression = TILESET_COMP_RLE,
    };

    err = load_tiles(&vctx, &options);
    if (err) exit(1);

    gfx_tilemap_place(&vctx, TILE_WAVEFORM + waveform, 1, WAVEFORM_X, WAVEFORM_Y);
    gfx_tilemap_place(&vctx, TILE_OCTAVE + octave, 1, OCTAVE_X, OCTAVE_Y);

    sprite_record.flags = SPRITE_NONE;
    sprite_record.tile = TILE_RECORD + 1;
    sprite_record.x = RECORD_X;
    sprite_record.y = RECORD_Y;
    err = gfx_sprite_render(&vctx, RECORD_INDEX, &sprite_record);
    if(err) exit(4);

    for (uint8_t i = 0; i < MAX_NOTES; i++) {
        Note *current = &NOTES[i];
        uint8_t o = 2 - current->octave;
        uint16_t freq = NOTE_TO_FREQ(current->note);
        freq = freq >> (2 * o);
        current->freq = freq;
    }

    sound_set(0, WAV_SAWTOOTH);
    sound_set(1, WAV_SQUARE);
    sound_set(2, WAV_TRIANGLE);
    sound_set(3, WAV_SAWTOOTH);
    sound_init();

    gfx_enable_screen(1);
}

void deinit(void) {
    gfx_error err;  // TODO: handle errors
    err = sound_deinit();

    // deinit sprites
    // for(uint8_t i = 0; i < PLAYER_MAX_WIDTH + 2; i++) {
    //     err = gfx_sprite_set_tile(&vctx, player.sprite_index+i, EMPTY_TILE);
    // }
    // err = gfx_sprite_set_tile(&vctx, ball.sprite_index, EMPTY_TILE);

    err = ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
}

static uint8_t keys[36];

static Note *playing[MAX_VOICES];
static uint8_t released = 0;

void keypress(uint8_t note, uint8_t octave) {
    (void *)released;

    uint16_t index = note + (octave * 12);
    if (index >= MAX_NOTES) return;  // ignore it

    Note *played = &NOTES[index];
    int8_t free = -1;

    uint8_t i = 0;
    for (i = 0; i < MAX_VOICES; i++) {
        Note *current = playing[i];
        if(!released && current == NULL) {
            free = i;
            break;
        }

        if (current->note == played->note && current->octave == played->octave) {
            playing[i] = NULL;  // remove it
        }
    }

    if(free >= 0) {
        playing[i] = played;
    }
}

void set_octave(uint8_t o) {
    octave = o;
    gfx_tilemap_place(&vctx, TILE_OCTAVE + o, 1, OCTAVE_X, OCTAVE_Y);
}

void set_waveform(uint8_t w) {
    gfx_tilemap_place(&vctx, TILE_WAVEFORM + waveform, 1, WAVEFORM_X, WAVEFORM_Y);

    sound_stop_all();
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        sound_set(i, w);
    }
}

uint8_t input(void) {
    uint16_t input = 0;
    uint16_t size = sizeof(keys);
    zos_err_t err = read(DEV_STDIN, keys, &size);
    if (err != ERR_SUCCESS) {
        printf("Failed to read from DEV_STDIN\n");
        exit(3);
    }

    // TODO: map the snes controller for simple playback?
    // if(controller_mode == 1) {
    //     input |= controller_read();
    // }

    // if(input & BUTTON_START ) return ACTION_PAUSE;
    // if(input & BUTTON_SELECT ) return ACTION_QUIT;
    // if(input & BUTTON_B) {
    //     // freqs[voice] = NOTE_FREQ(NOTE_C);
    //     playing[voice] = KEY_TO_NOTE(NOTE_C, 0);
    //     voice++;
    // }

    uint8_t i = 0;
    released = 0;
    for (i = 0; i < size; i++) {
        uint8_t k = keys[i];

        // end of the buffer
        if (k == 0)
            break;

        if (k == KB_RELEASED) {
            released = 1;
            continue;
        }

        if (!released) {
            switch (k) {
                /* PROGRAM */
                case KB_ESC:
                    return ACTION_QUIT;
                    break;

                /* STOP ALL */
                case KB_KEY_TAB:
                    playing[0] = NULL;
                    playing[1] = NULL;
                    playing[2] = NULL;
                    playing[3] = NULL;
                    break;

                /* WAVEFORM */
                case KB_UP_ARROW:
                    waveform++;
                    if (waveform > WAVEFORM_MAX) waveform = WAVEFORM_MIN;
                    set_waveform(waveform);
                    break;
                case KB_DOWN_ARROW:
                    waveform--;
                    if (waveform > WAVEFORM_MAX) waveform = WAVEFORM_MAX;
                    set_waveform(waveform);
                    break;

                /* OCTAVE */
                case '1':
                    set_octave(0);
                    break;
                case '2':
                    set_octave(1);
                    break;
                case '3':
                    set_octave(2);
                    break;

                case 'z':  // octave down
                    octave--;
                    if (octave > MAX_OCTAVE) octave = 0;
                    set_octave(octave);
                    break;

                case 'x':  // octave up
                    octave++;
                    if (octave > MAX_OCTAVE) octave = MAX_OCTAVE;
                    set_octave(octave);
                    break;
            }
        }

        switch (k) {
            /* NOTES */
            case KEY_C:
                keypress(NOTE_C, octave);
                break;

            case KEY_Cs:
                keypress(NOTE_Cs, octave);
                break;

            case KEY_D:
                keypress(NOTE_D, octave);
                break;

            case KEY_Ds:
                keypress(NOTE_Ds, octave);
                break;

            case KEY_E:
                keypress(NOTE_E, octave);
                break;

            case KEY_F:
                keypress(NOTE_F, octave);
                break;

            case KEY_Fs:
                keypress(NOTE_Fs, octave);
                break;

            case KEY_G:
                keypress(NOTE_G, octave);
                break;

            case KEY_Gs:
                keypress(NOTE_Gs, octave);
                break;

            case KEY_A:
                keypress(NOTE_A, octave);
                break;

            case KEY_As:
                keypress(NOTE_As, octave);
                break;

            case KEY_B:
                keypress(NOTE_B, octave);
                break;

            case KEY_C2:
                keypress(NOTE_C, octave + 1);
                break;

            case KEY_Cs2:
                keypress(NOTE_Cs, octave + 1);
                break;

            case KEY_D2:
                keypress(NOTE_D, octave + 1);
                break;
        }

        released = 0;
    }

    return ACTION_NONE;
}

void load_tilemap(void) {
    uint8_t line[WIDTH];
    uint8_t line_l1[WIDTH];
    uint8_t *tilemap = get_tilemap_start();
    uint8_t *tilemap_l1 = get_tilemap_l1_start();

    // Load the tilemap
    for (uint16_t row = 0; row < HEIGHT; row++) {
        uint16_t offset = row * WIDTH;
        for (uint16_t col = 0; col < WIDTH; col++) {
            line[col] = tilemap[offset + col] + TILEMAP_OFFSET;
            line_l1[col] = tilemap_l1[offset + col] + TILEMAP_OFFSET;
        }
        // memcpy(&tiles[offset], &line, WIDTH);
        gfx_tilemap_load(&vctx, line, WIDTH, 0, 0, row);
        gfx_tilemap_load(&vctx, line_l1, WIDTH, 1, 0, row);
    }
}

void update(void) {
    uint8_t i = 0;
    for (i = 0; i < MAX_VOICES; i++) {
        Note *current = playing[i];
        if (current == NULL) continue;
        sound_play(i, current->freq, 2);
    }
}

void draw(void) {
    gfx_wait_vblank(&vctx);
    // ...
    gfx_wait_end_vblank(&vctx);
}
