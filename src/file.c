#include <stdio.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include "file.h"

static void print_record(Record *record) {
    uint16_t frame = record->frame;
    uint8_t freq = record->freq;
    uint8_t voice_wave = record->voice_wave;

    if(freq == 0xFF && voice_wave == 0xFF) {
      printf("END (%d)\n\n", frame);
      return;
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

zos_err_t file_save(Track *track) {
  (void *)track;

  const char* filename = "B:/piano.ptz";
  zos_dev_t dev = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
  if(dev < 0) {
    printf("Could not open '%s'\n", filename);
  }

  uint16_t position = 0;
  uint16_t filesize = 0;
  while(1) {
    Record *record = track_at(position);
    if(record == NULL) break;

    uint16_t size = sizeof(Record);
    zos_err_t err = write(dev, record, &size);
    if(err != ERR_SUCCESS) {
      printf("Failed to write record at position %d (%d)\n", position, err);
      break;
    }

    if(size != sizeof(Record)) {
      printf("Failed to write record at position %d, wrote %d bytes\n", position, size);
      break;
    }

    position++;
    filesize+=size;

    if((position > MAX_RECORDS) || (track_end(record))) {
      break;
    }
  }

  // at the end, close the file and leave
  close(dev);

  printf("Wrote %d records to '%s' (%d bytes)\n", position, filename, filesize);
  return ERR_SUCCESS;
}

zos_err_t file_load(const char* path, Track *track) {
  (void *)path;
  (void *)track;
  const char* filename = "B:/piano.ptz";
  zos_dev_t dev = open(filename, O_RDONLY);
  if(dev < 0) {
    // failed to open
    return -dev;
  }

  uint16_t position = 0;
  track_init();

  while(1) {
    Record record;
    uint16_t size = sizeof(Record);
    zos_err_t err = read(dev, &record, &size);
    if(err != ERR_SUCCESS) {
      printf("Failed to load record at position %d\n", position);
      return err;
    }

    if(size == 0) {
      close(dev);
      break;
    } else if(size != sizeof(Record)) {
      printf("Failed to read record at position %d, read %d bytes", position, size);
      return ERR_ENTRY_CORRUPTED;
    }

    // print_record(&record);

    track_store(&record);

    if(track_end(&record)) {
      close(dev);
      break;
    }
  }

  return ERR_SUCCESS;
}