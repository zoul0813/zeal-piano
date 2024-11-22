#include <stdint.h>
#include <zvb_sound.h>

typedef struct {
  uint16_t note;
  uint16_t freq;
} Note;

const static uint16_t OCTAVE5[] = {
    SOUND_FREQ_TO_DIV(1047), // C
    SOUND_FREQ_TO_DIV(1109), // C# / Db
    SOUND_FREQ_TO_DIV(1175), // D
    SOUND_FREQ_TO_DIV(1245), // D# / Eb
    SOUND_FREQ_TO_DIV(1319), // E
    SOUND_FREQ_TO_DIV(1397), // F
    SOUND_FREQ_TO_DIV(1480), // F# / Gb
    SOUND_FREQ_TO_DIV(1568), // G
    SOUND_FREQ_TO_DIV(1661), // G# / Ab
    SOUND_FREQ_TO_DIV(1760), // A
    SOUND_FREQ_TO_DIV(1865), // A# / Bb
    SOUND_FREQ_TO_DIV(1976), // B
};

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

#define MAX_OCTAVE  2

static Note NOTES[] = {
    /* 0 */ {
        .note = NOTE_C,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(131),
    },
    /* 1 */ {
        .note = NOTE_Cs,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(139),
    },
    /* 2 */ {
        .note = NOTE_D,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(147),
    },
    /* 3 */ {
        .note = NOTE_Ds,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(155),
    },
    /* 4 */ {
        .note = NOTE_E,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(165),
    },
    /* 5 */ {
        .note = NOTE_F,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(175),
    },
    /* 6 */ {
        .note = NOTE_Fs,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(185),
    },
    /* 7 */ {
        .note = NOTE_G,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(196),
    },
    /* 8 */ {
        .note = NOTE_Gs,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(208),
    },
    /* 9 */ {
        .note = NOTE_A,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(220),
    },
    /* 10 */ {
        .note = NOTE_As,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(233),
    },
    /* 11 */ {
        .note = NOTE_B,
        // .octave = 3,
        .freq = SOUND_FREQ_TO_DIV(247),
    },
    /* 12 */ {
        .note = NOTE_C,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(262),
    },
    /* 13 */ {
        .note = NOTE_Cs,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(277),
    },
    /* 14 */ {
        .note = NOTE_D,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(294),
    },
    /* 15 */ {
        .note = NOTE_Ds,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(311),
    },
    /* 16 */ {
        .note = NOTE_E,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(330),
    },
    /* 17 */ {
        .note = NOTE_F,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(349),
    },
    /* 18 */ {
        .note = NOTE_Fs,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(370),
    },
    /* 19 */ {
        .note = NOTE_G,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(392),
    },
    /* 20 */ {
        .note = NOTE_Gs,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(415),
    },
    /* 21 */ {
        .note = NOTE_A,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(440),
    },
    /* 22 */ {
        .note = NOTE_As,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(466),
    },
    /* 23 */ {
        .note = NOTE_B,
        // .octave = 4,
        .freq = SOUND_FREQ_TO_DIV(494),
    },
    /* 24 */ {
        .note = NOTE_C,
        // .octave = 5,
        .freq = SOUND_FREQ_TO_DIV(523),
    },
    /* 25 */ {
        .note = NOTE_Cs,
        // .octave = 5,
        .freq = SOUND_FREQ_TO_DIV(554),
    },
    /* 26 */ {
        .note = NOTE_D,
        .freq = SOUND_FREQ_TO_DIV(587),
        // .octave = 5,
    },
    /* 27 */ {
        .note = NOTE_Ds,
        .freq = SOUND_FREQ_TO_DIV(622),
        // .octave = 5,
    },
    /* 28 */ {
        .note = NOTE_E,
        .freq = SOUND_FREQ_TO_DIV(659),
        // .octave = 5,
    },
    /* 29 */ {
        .note = NOTE_F,
        .freq = SOUND_FREQ_TO_DIV(699),
        // .octave = 5,
    },
    /* 30 */ {
        .note = NOTE_Fs,
        .freq = SOUND_FREQ_TO_DIV(740),
        // .octave = 5,
    },
    /* 31 */ {
        .note = NOTE_G,
        .freq = SOUND_FREQ_TO_DIV(784),
        // .octave = 5,
    },
    /* 32 */ {
        .note = NOTE_Gs,
        .freq = SOUND_FREQ_TO_DIV(831),
        // .octave = 5,
    },
    /* 33 */ {
        .note = NOTE_A,
        .freq = SOUND_FREQ_TO_DIV(880),
        // .octave = 5,
    },
    /* 34 */ {
        .note = NOTE_As,
        .freq = SOUND_FREQ_TO_DIV(932),
        // .octave = 5,
    },
    /* 35 */ {
        .note = NOTE_B,
        .freq = SOUND_FREQ_TO_DIV(988),
        // .octave = 5,
    },
};
static uint8_t MAX_NOTES = DIM(NOTES); // sizeof(NOTES) / sizeof(Note);
// static uint8_t MAX_NOTES = sizeof(NOTES) / sizeof(Note);

#define KEY_TO_NOTE(key, octave) &NOTES[key + (octave * 12)]
#define NOTE_TO_FREQ(note) OCTAVE5[note]
#define NOTE_FREQ(note) OCTAVE5[note];

#define IS_SIXTEENTH(frames) ((frames % 8) == 0)
#define IS_EIGHTH(frames) ((frames % 16) == 0)
#define IS_QUARTER(frames) ((frames % 32) == 0)
#define IS_HALF(frames) ((frames % 64) == 0)
#define IS_WHOLE(frames) ((frames % 128) == 0)