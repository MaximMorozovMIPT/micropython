#ifndef MICROPY_PORTS_JOS_LEXER_H
#define MICROPY_PORTS_JOS_LEXER_H

#include "py/builtin.h"
#include "py/lexer.h"
#include "py/reader.h"

mp_lexer_t *mp_lexer_new_from_fd(qstr filename, int fd, bool close_fd);
mp_lexer_t *mp_lexer_new_from_file(qstr filename);

#endif  // MICROPY_PORTS_JOS_LEXER_H
