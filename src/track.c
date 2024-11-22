#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "track.h"

static Track track;
static uint16_t track_position;

Track* track_get(void) {
  return &track;
}

void track_init(void) {
  track.state = T_NONE;
  track_position = 0;
  track.length = 0;

  // TODO: can we just initialize the first element as an "END" record?
  // track.records[0].freq = 0xFF;
  // track.records[0].voice_wave = 0xFF;

  for(uint16_t i = 0; i < MAX_RECORDS; i++) {
    Record record = { 0, 0xFF, 0xFF };
    memcpy(&track.records[i], &record, sizeof(Record));
  }
}

void track_transport(track_state_t state, uint16_t frame) {
  // if we're recordiong, or playing, we stop
  if(state == track.state) state = T_NONE;

  switch(track.state) {
    case T_RECORD:
      // add the "END" record
      Record record = {
          .frame = frame,
          .freq = 0xFF,
          .voice_wave = 0xFF,
      };
      track_store(&record);
      break;
  }

  // track_position = 0; // maybe
  track.state = state;

  switch(state) {
    case T_PLAY:
      if(frame > 0) {
        Record *prev = NULL;
        for(uint16_t i = 0; i < MAX_RECORDS; i++) {
          Record *record = track_at(i);
          // found a matching frame
          if(record->frame == frame) {
            track_position = i;
            break;
          }
          // requested frame is between prev/next, so use prev
          if(record->frame > frame && prev != NULL) {
            track_position = i--; //prev->frame;
            break;
          }
          // // we found the last record, so stop
          if(track_end(record)) {
            track_position = i;
            break;
          }
          prev = record;
        }
      } else {
        track_position = 0;
      }
      break;
  }
}

track_state_t track_state(void) {
  return track.state;
}

Record* track_next(uint8_t tick) {
  Record *record = &track.records[track_position];
  if(tick) track_tick();
  return record;
}

Record* track_at(uint16_t position) {
  if(position < track.length) {
    return &track.records[position];
  }
  return NULL;
}

void track_store(Record *record) {
  memcpy(&track.records[track_position], record, sizeof(Record));
  track_tick();
  track.length = track_position;
}

void track_tick(void) {
  track_position++;
  if(track_position >= MAX_RECORDS) {
    track_position = MAX_RECORDS - 1;
    uint16_t frame = track.records[track_position].frame;
    track_transport(T_NONE, frame);
  }
}

uint16_t track_length(void) {
  return track.length;
}

uint16_t track_pos(void) {
  return track_position;
}


void track_print(void) {
  printf("Track: %d records, %d position\n", track.length, track_position);
  printf("Track: %d records, %d position\n", track_length(), track_pos());
  track_position = 0;

  printf("-------------------------\n");
  printf("FRAME FREQUENCY VO WF __ \n");
  printf("-------------------------\n");
  for(uint16_t i = 0; i < track.length; i++) {
    Record *record = track_next(1);
    uint16_t frame = record->frame;
    uint8_t freq = record->freq;
    uint8_t voice_wave = record->voice_wave;

    if(freq == 0xFF && voice_wave == 0xFF) {
      printf("END (%d)\n\n", frame);
      break;
    }

    uint8_t voice = ((voice_wave & 0xF0) >> 4);
    uint8_t wave = (voice_wave & 0x0F);
    // printf("%05d  %04d  %02d  %02d %02x\n", frame, freq, voice, wave, voice_wave);
    printf("%05d ", frame);
    if(freq > 0) {
      printf(" on(");
    } else {
      printf("off(");
    }
    printf("%04d) %02d %02d ", freq, voice, wave);
    printf("%02x", voice_wave);
    printf("\n");
  }
}
