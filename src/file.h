#include <zos_errors.h>
#include <zgdk/sound/music.h>

#ifndef FILE_H
#define FILE_H

zos_err_t file_set(const char* path);
const char* file_get(void);
zos_err_t file_save(Track *track);
zos_err_t file_load(const char* path, Track *track);

#endif