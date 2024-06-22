#include "lexer.h"

#include <lib.h>

#include "py/lexer.h"
#include "py/builtin.h"
#include "py/repl.h"
#include "py/objstr.h"
#include "extmod/misc.h"
#include "extmod/modplatform.h"
#include "genhdr/mpversion.h"

mp_lexer_t *mp_lexer_new_from_fd(qstr filename, int fd, bool close_fd) {
    mp_reader_t reader;
    mp_reader_new_file_from_fd(&reader, fd, close_fd);
    return mp_lexer_new(filename, reader);
}

mp_lexer_t *mp_lexer_new_from_file(qstr filename) {
    mp_reader_t reader;
    mp_reader_new_file(&reader, filename);
    return mp_lexer_new(filename, reader);
}
