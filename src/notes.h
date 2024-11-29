#include <stdint.h>
#include <zvb_sound.h>

typedef struct {
    uint16_t note;
    uint16_t freq;
} Note;

typedef enum {
    PK_NONE     = 0,
    PK_WHITE    = 1,
    PK_BLACK    = 2,
    PK_PRESSED  = 3,
} KeyType;

typedef struct {
    uint8_t x;
    uint8_t y;
    KeyType type;
} PianoKey;

#define NOTE_C      0
#define NOTE_Cs     1
#define NOTE_Db     1
#define NOTE_D      2
#define NOTE_Ds     3
#define NOTE_Eb     3
#define NOTE_E      4
#define NOTE_F      5
#define NOTE_Fs     6
#define NOTE_Gb     6
#define NOTE_G      7
#define NOTE_Gs     8
#define NOTE_Ab     8
#define NOTE_A      9
#define NOTE_As     10
#define NOTE_Bb     10
#define NOTE_B      11

#define KEY_C       'a'
#define KEY_Cs      'w'
#define KEY_D       's'
#define KEY_Ds      'e'
#define KEY_E       'd'
#define KEY_F       'f'
#define KEY_Fs      't'
#define KEY_G       'g'
#define KEY_Gs      'y'
#define KEY_A       'h'
#define KEY_As      'u'
#define KEY_B       'j'
#define KEY_C2      'k'
#define KEY_Cs2     'o'
#define KEY_D2      'l'

#define MAX_OCTAVE 2

#define ONSCREEN_KEYS   15U
#define MAX_NOTES       36U

static PianoKey KEYS[] = {
    { .type = PK_WHITE, .x = 2, .y = 6 },
    { .type = PK_BLACK, .x = 3, .y = 5 },
    { .type = PK_WHITE, .x = 4, .y = 6 },
    { .type = PK_BLACK, .x = 5, .y = 5 },
    { .type = PK_WHITE, .x = 6, .y = 6 },
    { .type = PK_WHITE, .x = 7, .y = 6 },
    { .type = PK_BLACK, .x = 8, .y = 5 },
    { .type = PK_WHITE, .x = 9, .y = 6 },
    { .type = PK_BLACK, .x = 10, .y = 5 },
    { .type = PK_WHITE, .x = 11, .y = 6 },
    { .type = PK_BLACK, .x = 12, .y = 5 },
    { .type = PK_WHITE, .x = 13, .y = 6 },
    { .type = PK_WHITE, .x = 14, .y = 6 },
    { .type = PK_BLACK, .x = 15, .y = 5 },
    { .type = PK_WHITE, .x = 16, .y = 6 },
};
// static const uint8_t ONSCREEN_KEYS = DIM(KEYS);

static Note NOTES[] = {
    /* OCTAVE 3*/
    { .note = NOTE_C, .freq = SOUND_FREQ_TO_DIV(131) },
    { .note = NOTE_Cs, .freq = SOUND_FREQ_TO_DIV(139) },
    { .note = NOTE_D, .freq = SOUND_FREQ_TO_DIV(147) },
    { .note = NOTE_Ds, .freq = SOUND_FREQ_TO_DIV(155) },
    { .note = NOTE_E, .freq = SOUND_FREQ_TO_DIV(165) },
    { .note = NOTE_F, .freq = SOUND_FREQ_TO_DIV(175) },
    { .note = NOTE_Fs, .freq = SOUND_FREQ_TO_DIV(185) },
    { .note = NOTE_G, .freq = SOUND_FREQ_TO_DIV(196) },
    { .note = NOTE_Gs, .freq = SOUND_FREQ_TO_DIV(208) },
    { .note = NOTE_A, .freq = SOUND_FREQ_TO_DIV(220) },
    { .note = NOTE_As, .freq = SOUND_FREQ_TO_DIV(233) },
    { .note = NOTE_B, .freq = SOUND_FREQ_TO_DIV(247) },
    /* OCTAVE 4 */
    { .note = NOTE_C, .freq = SOUND_FREQ_TO_DIV(262) },
    { .note = NOTE_Cs, .freq = SOUND_FREQ_TO_DIV(277) },
    { .note = NOTE_D, .freq = SOUND_FREQ_TO_DIV(294) },
    { .note = NOTE_Ds, .freq = SOUND_FREQ_TO_DIV(311) },
    { .note = NOTE_E, .freq = SOUND_FREQ_TO_DIV(330) },
    { .note = NOTE_F, .freq = SOUND_FREQ_TO_DIV(349) },
    { .note = NOTE_Fs, .freq = SOUND_FREQ_TO_DIV(370) },
    { .note = NOTE_G, .freq = SOUND_FREQ_TO_DIV(392) },
    { .note = NOTE_Gs, .freq = SOUND_FREQ_TO_DIV(415) },
    { .note = NOTE_A, .freq = SOUND_FREQ_TO_DIV(440) },
    { .note = NOTE_As, .freq = SOUND_FREQ_TO_DIV(466) },
    { .note = NOTE_B, .freq = SOUND_FREQ_TO_DIV(494) },
    /* OCTAVE 5 */
    { .note = NOTE_C, .freq = SOUND_FREQ_TO_DIV(523) },
    { .note = NOTE_Cs, .freq = SOUND_FREQ_TO_DIV(554) },
    { .note = NOTE_D, .freq = SOUND_FREQ_TO_DIV(587) },
    { .note = NOTE_Ds, .freq = SOUND_FREQ_TO_DIV(622) },
    { .note = NOTE_E, .freq = SOUND_FREQ_TO_DIV(659) },
    { .note = NOTE_F, .freq = SOUND_FREQ_TO_DIV(699) },
    { .note = NOTE_Fs, .freq = SOUND_FREQ_TO_DIV(740) },
    { .note = NOTE_G, .freq = SOUND_FREQ_TO_DIV(784) },
    { .note = NOTE_Gs, .freq = SOUND_FREQ_TO_DIV(831) },
    { .note = NOTE_A, .freq = SOUND_FREQ_TO_DIV(880) },
    { .note = NOTE_As, .freq = SOUND_FREQ_TO_DIV(932) },
    { .note = NOTE_B, .freq = SOUND_FREQ_TO_DIV(988) },
};
// static const uint8_t MAX_NOTES = DIM(NOTES);

#define KEY_TO_NOTE(key, octave) &NOTES[key + (octave * 12)]
#define NOTE_TO_FREQ(note) OCTAVE5[note]
#define NOTE_FREQ(note) OCTAVE5[note];

#define IS_SIXTEENTH(frames) ((frames % 8) == 0)
#define IS_EIGHTH(frames) ((frames % 16) == 0)
#define IS_QUARTER(frames) ((frames % 32) == 0)
#define IS_HALF(frames) ((frames % 64) == 0)
#define IS_WHOLE(frames) ((frames % 128) == 0)