#include <stdio.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include "file.h"

char* filename;

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

zos_err_t file_set(const char* path) {
  filename = path;
  return ERR_SUCCESS;
}

const char* file_get(void) {
  return filename;
}

zos_err_t file_save(Track *track) {
  (void *)track;

  // const char* filename = "B:/piano.ptz";
  zos_dev_t dev = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
  if(dev < 0) {
    printf("Could not open '%s'\n", filename);
  }

  uint16_t size = sizeof(uint16_t);
  zos_err_t err = write(dev, &track->length, &size);
  if(err != ERR_SUCCESS) {
    printf("Failed to write header, wrote %d with error %d (%02x)\n", size, err, err);
    return err;
  }

  size = sizeof(Record) * track->length;
  err = write(dev, track->records, &size);
  if(err != ERR_SUCCESS) {
    printf("Failed to write records, wrote %d with error %d (%02x)\n", size, err, err);
    return err;
  }

  // at the end, close the file and leave
  close(dev);

  printf("Wrote %d records to '%s' (%d bytes)\n", track->length, filename, size + 2);
  return ERR_SUCCESS;
}

zos_err_t file_load(const char* path, Track *track) {
  (void *)path;
  (void *)track;
  // const char* filename = "B:/piano.ptz";
  zos_dev_t dev = open(filename, O_RDONLY);
  if(dev < 0) {
    // failed to open
    return -dev;
  }

  uint16_t position = 0;
  track_init();

  uint16_t size = sizeof(uint16_t);
  zos_err_t err = read(dev, &track->length, &size);
  if(err != ERR_SUCCESS) {
    printf("Failed to load header, read %d with error %d (%02x)\n", size, err, err);
    return err;
  }

  size = sizeof(Record) * track->length;
  err = read(dev, track->records, &size);
  if(err != ERR_SUCCESS) {
    printf("Failed to load records, read %d with %d (%02x)\n", size, err, err);
    return err;
  }

  close(dev);

  return ERR_SUCCESS;
}