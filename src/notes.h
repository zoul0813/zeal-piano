#include <stdint.h>
#include <zvb_sound.h>

typedef struct {
  uint16_t note;
  uint16_t freq;
  uint8_t octave;
//   uint8_t playing;
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

static Note NOTES[36] = {
    /* 0 */ {
        .note = NOTE_C,
        .octave = 0,
    },
    /* 1 */ {
        .note = NOTE_Cs,
        .octave = 0,
    },
    /* 2 */ {
        .note = NOTE_D,
        .octave = 0,
    },
    /* 3 */ {
        .note = NOTE_Ds,
        .octave = 0,
    },
    /* 4 */ {
        .note = NOTE_E,
        .octave = 0,
    },
    /* 5 */ {
        .note = NOTE_F,
        .octave = 0,
    },
    /* 6 */ {
        .note = NOTE_Fs,
        .octave = 0,
    },
    /* 7 */ {
        .note = NOTE_G,
        .octave = 0,
    },
    /* 8 */ {
        .note = NOTE_Gs,
        .octave = 0,
    },
    /* 9 */ {
        .note = NOTE_A,
        .octave = 0,
    },
    /* 10 */ {
        .note = NOTE_As,
        .octave = 0,
    },
    /* 11 */ {
        .note = NOTE_B,
        .octave = 0,
    },
    /* 12 */ {
        .note = NOTE_C,
        .octave = 1,
    },
    /* 13 */ {
        .note = NOTE_Cs,
        .octave = 1,
    },
    /* 14 */ {
        .note = NOTE_D,
        .octave = 1,
    },
    /* 15 */ {
        .note = NOTE_Ds,
        .octave = 1,
    },
    /* 16 */ {
        .note = NOTE_E,
        .octave = 1,
    },
    /* 17 */ {
        .note = NOTE_F,
        .octave = 1,
    },
    /* 18 */ {
        .note = NOTE_Fs,
        .octave = 1,
    },
    /* 19 */ {
        .note = NOTE_G,
        .octave = 1,
    },
    /* 20 */ {
        .note = NOTE_Gs,
        .octave = 1,
    },
    /* 21 */ {
        .note = NOTE_A,
        .octave = 1,
    },
    /* 22 */ {
        .note = NOTE_As,
        .octave = 1,
    },
    /* 23 */ {
        .note = NOTE_B,
        .octave = 1,
    },
    /* 24 */ {
        .note = NOTE_C,
        .octave = 2,
    },
    /* 25 */ {
        .note = NOTE_Cs,
        .octave = 2,
    },
    /* 26 */ {
        .note = NOTE_D,
        .octave = 2,
    },
    /* 27 */ {
        .note = NOTE_Ds,
        .octave = 2,
    },
    /* 28 */ {
        .note = NOTE_E,
        .octave = 2,
    },
    /* 29 */ {
        .note = NOTE_F,
        .octave = 2,
    },
    /* 30 */ {
        .note = NOTE_Fs,
        .octave = 2,
    },
    /* 31 */ {
        .note = NOTE_G,
        .octave = 2,
    },
    /* 32 */ {
        .note = NOTE_Gs,
        .octave = 2,
    },
    /* 33 */ {
        .note = NOTE_A,
        .octave = 2,
    },
    /* 34 */ {
        .note = NOTE_As,
        .octave = 2,
    },
    /* 35 */ {
        .note = NOTE_B,
        .octave = 2,
    },
};
static uint8_t MAX_NOTES = sizeof(NOTES);

#define KEY_TO_NOTE(key, octave) &NOTES[key + (octave * 12)]
#define NOTE_TO_FREQ(note) OCTAVE5[note]
#define NOTE_FREQ(note) OCTAVE5[note];