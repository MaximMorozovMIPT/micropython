#ifndef MICROPY_PORTS_JOS_MODIO_H
#define MICROPY_PORTS_JOS_MODIO_H

#include "py/builtin.h"
#include "py/lexer.h"
#include "py/reader.h"

void mp_reader_new_file_from_fd(mp_reader_t *reader, int fd, bool close_fd);
void mp_reader_new_file(mp_reader_t *reader, qstr filename);
mp_import_stat_t mp_import_stat(const char *path);

#endif  // MICROPY_PORTS_JOS_MODIO_H
