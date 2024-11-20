#include <stdint.h>
#include <string.h>
#include "track.h"

static uint16_t _track_index = 0;
Track _track;

void track_init(void) {
  _track_index = 0;
  _track.state = TRACK_NONE;

  for(uint16_t i = 0; i < MAX_RECORDS; i++) {
    Record record = { 0, 0, 0xFF };
    memcpy(&_track.records[i], &record, sizeof(Record));
  }
}

void track_stop(void) {
  _track.state = TRACK_NONE;
}

void track_play(void) {
  _track.state = TRACK_PLAY;
}

void track_record(void) {
  if(_track.state != TRACK_RECORD) return;

  _track.state = TRACK_RECORD;
}

void track_store(Record *record) {
  memcpy(&_track.records[_track_index], record, sizeof(Record));
  track_tick();
}

void track_tick(void) {
  _track_index++;
  if(_track_index >= MAX_RECORDS) {
    track_stop();
  }
}

uint16_t track_index(void) {
  return _track_index;
}