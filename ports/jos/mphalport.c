/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <lib.h>
#include <random.h>
#include <time.h>

#include "py/mphal.h"
#include "py/mpthread.h"
#include "py/runtime.h"
#include "extmod/misc.h"

int mp_hal_stdin_rx_chr(void) {
    unsigned char c;
    ssize_t ret;

    MP_THREAD_GIL_EXIT();
    ret = read(STDIN_FILENO, &c, 1);
    MP_THREAD_GIL_ENTER();

    if (ret == 0) {
        c = 4; // EOF, ctrl-D
    } else if (c == '\n') {
        c = '\r';
    }
    return c;
}

mp_uint_t mp_hal_stdout_tx_strn(const char *str, size_t len) {
    MP_THREAD_GIL_EXIT();
    ssize_t ret = write(STDOUT_FILENO, str, len);
    MP_THREAD_GIL_ENTER();

    mp_uint_t written = ret < 0 ? 0 : ret;
    int dupterm_res = mp_os_dupterm_tx_strn(str, len);
    if (dupterm_res >= 0) {
        written = MIN((mp_uint_t)dupterm_res, written);
    }
    return written;
}

// cooked is same as uncooked because the terminal does some postprocessing
void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
    mp_hal_stdout_tx_strn(str, len);
}

void mp_hal_stdout_tx_str(const char *str) {
    mp_hal_stdout_tx_strn(str, strlen(str));
}

#ifndef mp_hal_ticks_ms
mp_uint_t mp_hal_ticks_ms(void) {
    int now = vsys_gettime();
    struct tm tnow;

    mktime(now, &tnow);
    return tnow.tm_sec * 1000;
}
#endif

#ifndef mp_hal_ticks_us
mp_uint_t mp_hal_ticks_us(void) {
    int now = vsys_gettime();
    struct tm tnow;

    mktime(now, &tnow);
    return tnow.tm_sec * 1000000;
}
#endif

#ifndef mp_hal_time_ns
uint64_t mp_hal_time_ns(void) {
    int now = vsys_gettime();
    struct tm tnow;

    mktime(now, &tnow);
    return tnow.tm_sec * 1000000000ULL;
}
#endif

#ifndef mp_hal_delay_ms
void mp_hal_delay_ms(mp_uint_t ms) {
    mp_uint_t start = mp_hal_ticks_ms();
    while (mp_hal_ticks_ms() - start < ms) {
        mp_event_wait_ms(1);
    }
}
#endif

void mp_hal_get_random(size_t n, void *buf) {
    int *int_buffer = (int *)buf;

    size_t reminder = n % sizeof(int);
    size_t ints_count = n / sizeof(int);
    size_t i = 0;
    for (; i < ints_count; ++i) {
        int_buffer[i] = rand();
    }

    char *char_buffer = (char *)(int_buffer + i);
    union {
        int r;
        char ch[sizeof(int)];
    } r;
    r.r = rand();
    for (i = 0; i < reminder; ++i) {
        char_buffer[i] = r.ch[i];
    }
}
