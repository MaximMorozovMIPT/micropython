#include "modio.h"

#include <lib.h>

#include "py/lexer.h"
#include "py/builtin.h"
#include "py/repl.h"
#include "py/objstr.h"
#include "extmod/misc.h"
#include "extmod/modplatform.h"
#include "genhdr/mpversion.h"

typedef struct _mp_reader_posix_t {
    bool close_fd;
    int fd;
    size_t len;
    size_t pos;
    byte buf[20];
} mp_reader_posix_t;

static mp_uint_t mp_reader_jos_readbyte(void *data) {
    mp_reader_posix_t *reader = (mp_reader_posix_t *)data;
    if (reader->pos >= reader->len) {
        if (reader->len == 0) {
            return MP_READER_EOF;
        } else {
            MP_THREAD_GIL_EXIT();
            int n = read(reader->fd, reader->buf, sizeof(reader->buf));
            MP_THREAD_GIL_ENTER();
            if (n <= 0) {
                reader->len = 0;
                return MP_READER_EOF;
            }
            reader->len = n;
            reader->pos = 0;
        }
    }
    return reader->buf[reader->pos++];
}

static void mp_reader_jos_close(void *data) {
    mp_reader_posix_t *reader = (mp_reader_posix_t *)data;
    if (reader->close_fd) {
        MP_THREAD_GIL_EXIT();
        close(reader->fd);
        MP_THREAD_GIL_ENTER();
    }
    m_del_obj(mp_reader_posix_t, reader);
}

void mp_reader_new_file_from_fd(mp_reader_t *reader, int fd, bool close_fd) {
    mp_reader_posix_t *rp = m_new_obj(mp_reader_posix_t);
    rp->close_fd = close_fd;
    rp->fd = fd;
    MP_THREAD_GIL_EXIT();
    int n = read(rp->fd, rp->buf, sizeof(rp->buf));
    if (n == -1) {
        if (close_fd) {
            close(fd);
        }
        MP_THREAD_GIL_ENTER();
        mp_raise_OSError(1);
    }
    MP_THREAD_GIL_ENTER();
    rp->len = n;
    rp->pos = 0;
    reader->data = rp;
    reader->readbyte = mp_reader_jos_readbyte;
    reader->close = mp_reader_jos_close;
}

void mp_reader_new_file(mp_reader_t *reader, qstr filename) {
    MP_THREAD_GIL_EXIT();
    int fd = open(qstr_str(filename), O_RDONLY);
    MP_THREAD_GIL_ENTER();
    if (fd < 0) {
        mp_raise_OSError_with_filename(1, qstr_str(filename));
    }
    mp_reader_new_file_from_fd(reader, fd, true);
}

mp_import_stat_t mp_import_stat(const char *path) {
    struct Stat statbuf;
    int err = stat(path, &statbuf);
    if (err < 0) {
        return MP_IMPORT_STAT_NO_EXIST;
    }
    return MP_IMPORT_STAT_FILE;
}
