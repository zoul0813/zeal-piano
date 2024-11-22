#include <stdint.h>

#ifdef __SDCC_VERSION_MAJOR
#include <zgdk.h>
#endif

#ifndef TRACK_H
#define TRACK_H


// 16k / 4
#define MAX_RECORDS   4096

typedef struct {
  uint16_t frame;
  uint16_t freq;
  uint8_t voice_wave;
} __attribute__((packed)) Record;

// #define TRACK_NONE    0
// #define TRACK_RECORD  1
// #define TRACK_PLAY    2

#define FREQ_NONE           0
#define FILENAME_MAX_LEN    16 /* ZealFS */

typedef enum {
  T_NONE    = 0,
  T_PLAY    = 1,
  T_RECORD  = 2,
} track_state_t;

typedef struct {
  track_state_t state;
  uint16_t length;
  Record records[MAX_RECORDS];
} __attribute__((packed)) Track;


Track* track_get(void);
void track_init(void);
void track_transport(track_state_t state, uint16_t frame);
// void track_record(void);
// void track_play(void);
// void track_stop(void);
void track_store(Record *record);
void track_tick(void);
track_state_t track_state(void);
Record* track_next(uint8_t tick);
Record* track_at(uint16_t position);

void track_print(void);

uint16_t track_length(void);
uint16_t track_pos(void);

static uint8_t track_get_voice(uint8_t voice_wave) {
  return (voice_wave & 0xF0) >> 4;
}

static uint8_t track_get_wave(uint8_t voice_wave) {
  return (voice_wave & 0x0F);
}

static uint8_t track_set_voice_wave(uint8_t voice, uint8_t wave) {
  return (voice << 4) | (wave);
}

// static void track_start(void) {
//   track_init();
// }

static uint8_t track_end(Record *record) {
  return (record->freq == 0xFF) && (record->voice_wave == 0xFF);
}

#endif