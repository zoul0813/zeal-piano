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
#include "file.h"

gfx_context vctx;
uint16_t frames = 0;
uint8_t controller_mode = 1;

uint16_t waveform = 0; // current waveform
uint8_t octave = 1; // current octave
int8_t volume = 3;

gfx_sprite sprite_record;
Track track;
// uint8_t sprite_record_frame = 0;

int main(int argc, char** argv) {
    // (void *)argc;
    // (void *)argv;

    if(argc == 1) {
        file_set(argv[0]);
    } else {
        file_set("b:/piano.ptz");
    }

    init();

    load_tilemap();
    set_volume(volume);
    music_load_from_file(file_get(), &track);

    while (true) {
        sound_loop();
        music_loop(0);
        uint8_t action = input();
        switch (action) {
            case ACTION_QUIT:
                goto quit_game;
        }

        if(music_state() == T_RECORD) {
            frames++;
        }

        update();
        draw();
    }

quit_game:
    deinit();

    // music_print();

    keyboard_flush();
    void* arg = (void*) (KB_READ_BLOCK | KB_MODE_COOKED);
    zos_err_t err = ioctl(DEV_STDIN, KB_CMD_SET_MODE, arg);
    if(err != ERR_SUCCESS) {
        printf("Failed to change keyboard mode %d (%02x)\n", err, err);
        exit(1);
    }

    char key[128] = { 0x00 };
    uint16_t size = 0;
    do {
        printf("Enter filename to save recording, press enter to quit without saving:\n");
        size = sizeof(key);
        zos_err_t err = read(DEV_STDIN, key, &size);
        if(err != ERR_SUCCESS) {
            printf("keyboard error: %d (%02x)\n", err, err);
        }

        if(size > 0) {
            switch(key[0]) {
                case KB_KEY_ENTER:
                    printf("File not saved\n");
                    break;
                default:
                    key[size - 1] = 0x00;
                    file_set(key);
                    file_save(&track);
            }
        }
    } while(size == 0);

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

    sound_init();
    for(uint8_t i = 0; i < MAX_VOICES; i++) {
        sound_set(i, waveform);
    }

    music_init(&track);

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
    gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_EMPTY);
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
static uint8_t playing_changed[MAX_VOICES] = { 0 };
static uint8_t released = 0;
static uint8_t key_state[15] = { 0 };

void keypress(uint8_t note, uint8_t octave) {
    uint16_t index = note + (octave * 12);
    if (index >= MAX_NOTES) return;  // ignore it

    /* If the key is already pressed and we still got a press, ignore */
    const uint8_t previous_state = key_state[note];
    const uint8_t new_state = !released;

    if (previous_state == new_state) {
        return;
    }

    key_state[note] = new_state;
    Note *played = &NOTES[index];
    int8_t free = -1;

    uint8_t i = 0;
    for (i = 0; i < MAX_VOICES; i++) {
        Note *current = playing[i];

        if((!released) && (current == NULL)) {
            free = i;
            break;
        }

        if (released && current == played) {
            playing[i] = NULL;  // remove it
            playing_changed[i] = 1;
            if(music_state() == T_RECORD) {
                Record record = {
                    .frame = frames,
                    .freq = FREQ_NONE,
                    .voice_wave = music_set_voice_wave(i, 0x0F), // (i << 4) | 0x0F,
                };
                music_store(&record);
            }
        }
    }

    if(free >= 0) {
        playing_changed[i] = 1;
        playing[i] = played;
        if(music_state() == T_RECORD) {
            Record record = {
                .frame = frames,
                .freq = played->freq,
                .voice_wave = music_set_voice_wave(free, waveform), // (free << 4) | (waveform),
            };
            music_store(&record);
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

void set_volume(int8_t v) {
    if(v < VOLUME_MIN) v = VOLUME_MIN;
    if(v > VOLUME_MAX) v = VOLUME_MAX;
    volume = v;

    for(int8_t i = VOLUME_MIN + 1; i < VOLUME_MAX + 1; i++) {
        uint8_t tile = TILE_EMPTY;
        if((volume > 0) && (volume >= i)) {
            tile = TILE_PROGRESS;
        }
        gfx_tilemap_place(&vctx, tile, 1, 2 + (i - 1), 12);
    }

    // char text[7];
    // sprintf(text, "V %02d", volume);
    // nprint_string(&vctx, text, 4, WIDTH - 4, 1);

    if(volume == 0) {
        zvb_sound_set_volume(VOL_0);
    } else {
        zvb_sound_set_volume(volume - 1);
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

        uint8_t state = music_state();
        if (!released) {
            switch (k) {
                /* PROGRAM */
                case KB_ESC:
                    return ACTION_QUIT;
                    break;

                /* Volume */
                case KB_KEY_MINUS:
                    volume--;
                    set_volume(volume);
                    break;
                case KB_KEY_EQUAL:
                    volume++;
                    set_volume(volume);
                    break;

                /* Record/Playback */
                case KB_KEY_SPACE:
                case KB_KEY_COMMA: // RECORD
                    if(state == T_NONE) {
                        frames = 0;
                        gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_RECORD);
                        music_transport(T_RECORD, frames);
                    } else {
                        gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_EMPTY);
                        music_transport(T_NONE, frames);
                    }
                    break;
                case KB_KEY_M: // PLAY
                    if(state == T_NONE) {
                        gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_PLAY);
                        music_transport(T_PLAY, 0);
                    } else {
                        music_transport(T_NONE, 0);
                        gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_EMPTY);
                    }
                    break;
                case KB_KEY_N: // NEW
                    frames = 0;
                    music_transport(T_NONE, frames);
                    music_init(&track);
                    gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_EMPTY);
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
    switch(music_state()) {
        case T_RECORD:
        case T_NONE:
            uint8_t i = 0;
            for (i = 0; i < MAX_VOICES; i++) {
                Note *current = playing[i];
                if (playing_changed[i] != 0) {
                    if (current == NULL) {
                        // FIXME: use stop function ?
                        sound_play(i, 0, 0xffff);
                    } else {
                        sound_play(i, current->freq, 0xffff);
                        playing_changed[i] = 0;
                    }
                }
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

    uint16_t len = music_length();
    uint16_t pos = music_pos();

    // 65536 - 5 bytes
    char text[7];
    sprintf(text, "L%05d", len);
    nprint_string(&vctx, text, 6, WIDTH - 7, 11);
    sprintf(text, "P%05d", pos);
    nprint_string(&vctx, text, 6, WIDTH - 7, 12);
    if(music_state() == T_PLAY) {
        sprintf(text, "F%05d", music_frame());
    } else {
        sprintf(text, "F%05d", frames);
    }
    nprint_string(&vctx, text, 6, WIDTH - 7, 13);

    switch(music_state()) {
        case T_RECORD:
            uint8_t frame = 0;
            // if(IS_EIGHTH(frames)) {
            //     frame = 0;
            // }
            if(IS_QUARTER(frames)) {
                frame = 2;
            }
            if(IS_WHOLE(frames)) {
                frame = 1;
            }
            gfx_sprite_set_tile(&vctx, STATE_INDEX, TILE_RECORD + frame);
            break;
    }

    gfx_wait_end_vblank(&vctx);
}
