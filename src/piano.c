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

uint16_t waveform = 0; // current waveform
uint8_t octave = 1; // current octave

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

        if(track_state() != TRACK_NONE) {
            frames++;
        }

        update();
        draw();
    }

quit_game:
    deinit();

    track_print();

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
    sprite_record.tile = TILE_EMPTY;
    sprite_record.x = STATE_X;
    sprite_record.y = STATE_Y;
    err = gfx_sprite_render(&vctx, STATE_INDEX, &sprite_record);
    if(err) exit(4);

    for (uint8_t i = 0; i < MAX_NOTES; i++) {
        Note *current = &NOTES[i];
        uint8_t o = 2 - current->octave;
        uint16_t freq = NOTE_TO_FREQ(current->note);
        freq = freq >> (2 * o);
        current->freq = freq;
    }

    sound_init();
    for(uint8_t i = 0; i < MAX_VOICES; i++) {
        sound_set(i, waveform);
    }

    track_init();

    ascii_map(' ', 1, TILE_EMPTY);
    ascii_map('0', 10, TILE_NUMBER);
    ascii_map('a', 26, 64);
    ascii_map('A', 26, 64);

    gfx_enable_screen(1);
}

void deinit(void) {
    gfx_error err;  // TODO: handle errors
    err = sound_deinit();

    // deinit sprites
    // for(uint8_t i = 0; i < PLAYER_MAX_WIDTH + 2; i++) {
    //     err = gfx_sprite_set_tile(&vctx, player.sprite_index+i, TILE_EMPTY);
    // }
    // err = gfx_sprite_set_tile(&vctx, ball.sprite_index, TILE_EMPTY);

    err = ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
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

static Note *playing[MAX_VOICES] = { NULL, NULL, NULL, NULL };
static uint8_t released = 0;

void keypress(uint8_t note, uint8_t octave) {
    uint16_t index = note + (octave * 12);
    if (index >= MAX_NOTES) return;  // ignore it

    Note *played = &NOTES[index];
    int8_t free = -1;

    uint8_t i = 0;
    for (i = 0; i < MAX_VOICES; i++) {
        Note *current = playing[i];

        if((!released) && (current == NULL)) {
            free = i;
            break;
        }

        if ((current->note == played->note) && (current->octave == played->octave)) {
            playing[i] = NULL;  // remove it
            if(track_state() == TRACK_RECORD) {
                Record record = {
                    .frame = frames,
                    .freq = 0x00,
                    .voice_wave = track_set_voice_wave(i, 0x0F), // (i << 4) | 0x0F,
                };
                track_store(&record);
            }
        }
    }

    if(free >= 0) {
        playing[i] = played;
        if(track_state() == TRACK_RECORD) {
            Record record = {
                .frame = frames,
                .freq = played->freq,
                .voice_wave = track_set_voice_wave(free, waveform), // (free << 4) | (waveform),
            };
            track_store(&record);
        }
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

static uint8_t keys[36];
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

                /* Record/Playback */
                case KB_KEY_SPACE:
                case KB_KEY_COMMA:
                    if(track_state() == TRACK_NONE) {
                        frames = 0;
                        gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_RECORD);
                        track_init();
                        track_record();
                    } else {
                        gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_EMPTY);
                        Record record = {
                            .frame = frames,
                            .freq = 0xFF,
                            .voice_wave = 0xFF,
                        };
                        track_store(&record);
                        track_stop();
                    }
                    break;
                case KB_KEY_M:
                    if(track_state() == TRACK_NONE) {
                        frames = 0;
                        gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_PLAY);
                        track_play();
                    } else {
                        track_stop();
                        gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_EMPTY);
                    }
                    break;
                case KB_KEY_N:
                    track_stop();
                    gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_EMPTY);
                    frames = 0;
                    track_init();
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

void update(void) {
    switch(track_state()) {
        case TRACK_PLAY:
            Record *record = track_next(false);
            if(record->frame == frames) {
                uint16_t frame = record->frame;
                uint8_t freq = record->freq;
                uint8_t voice_wave = record->voice_wave;
                uint8_t voice = track_get_voice(voice_wave); // ((voice_wave & 0xF0) >> 4);
                uint8_t wave = track_get_wave(voice_wave); // (voice_wave & 0x0F);

                if(voice_wave == 0xFF) {
                    track_stop();
                } else {
                    if(wave >= 0x0F) {
                        zvb_sound_set_voices((1 << voice), freq, WAV_SQUARE);
                    } else {
                        zvb_sound_set_voices((1 << voice), freq, wave);
                    }
                    track_tick();
                }
            }
            break;
        case TRACK_RECORD:
        case TRACK_NONE:
            uint8_t i = 0;
            for (i = 0; i < MAX_VOICES; i++) {
                Note *current = playing[i];
                if (current == NULL) continue;
                sound_play(i, current->freq, 2);
            }
            break;
    }
}

void draw(void) {
    gfx_wait_vblank(&vctx);

    uint8_t voices = 0;
    uint8_t i = 0;
    for(i = 0; i < MAX_VOICES; i++) {
        if(playing[i] != NULL) voices++;
    }
    if(voices > MAX_VOICES) {
        voices = 9;
    }
    gfx_tilemap_place(&vctx, TILE_NUMBER + voices, 1, VOICES_X, VOICES_Y);
    // gfx_tilemap_place(&vctx, TILE_NUMBER + track_state(), 1, VOICES_X + 2, VOICES_Y);

    uint16_t len = track_length();
    uint16_t pos = track_pos();

    // 65536 - 5 bytes
    char text[7];
    sprintf(text, "L %05d", len);
    nprint_string(&vctx, text, sizeof(text), WIDTH - 6, 11);
    sprintf(text, "P %05d", pos);
    nprint_string(&vctx, text, sizeof(text), WIDTH - 6, 12);
    sprintf(text, "F %05d", frames);
    nprint_string(&vctx, text, sizeof(text), WIDTH - 6, 13);

    if(IS_EIGHTH(frames)) {
        switch(track_state()) {
            case TRACK_RECORD:
                sprite_record_frame ^= 1;
                gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_RECORD + sprite_record_frame);
                break;
        }
    }

    gfx_wait_end_vblank(&vctx);
}
