//
// -- buffer.c
//
#include "buffer.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// -- ensure the buffer has space to add length chars
static void
reserve(buffer_t *buffer, int length) {
    if (buffer->end + length > buffer->capacity) {
        int new_capacity = (buffer->capacity < 64) ? 64 : 2 * buffer->capacity;
        while (buffer->end + length > new_capacity) {
            new_capacity = 2 * new_capacity;
        }
        if (!buffer->ptr) {
            buffer->ptr = malloc(new_capacity);
        } else {
            buffer->ptr = realloc(buffer->ptr, new_capacity);
        }
        buffer->capacity = new_capacity;
    }
}

//
// -- initialize the buffer
buffer_t *
buffer_init(buffer_t *buffer) {
    assert(buffer);
    buffer->begin = 0;
    buffer->end = 0;
    buffer->capacity = 0;
    buffer->ptr = NULL;
    buffer->current = 0;
    buffer->mark = 0;
    return buffer;
}

//
// -- free allocated memory
void
buffer_free(buffer_t *buffer) {
    assert(buffer);
    free(buffer->ptr);
    buffer->begin = 0;
    buffer->end = 0;
    buffer->capacity = 0;
    buffer->ptr = NULL;
    buffer->current = 0;
    buffer->mark = 0;
}

//
// -- reset contents
void
buffer_reset(buffer_t *buffer) {
    assert(buffer);
    buffer->begin = 0;
    buffer->end = 0;
    buffer->current = 0;
    buffer->mark = 0;
}

//
// -- buffer content size
int
buffer_size(buffer_t *buffer) {
    assert(buffer);
    return buffer->end - buffer->begin;
}

//
// -- append to the buffer
void
buffer_append(buffer_t *buffer, const char *ptr, int length) {
    assert(buffer);
    assert(ptr);
    reserve(buffer, length + 1);
    memcpy(buffer->ptr + buffer->end, ptr, length);
    buffer->end += length;
    buffer->ptr[buffer->end] = '\0';
}

void
buffer_append_ch(buffer_t *buffer, int ch) {
    assert(buffer);
    reserve(buffer, 2);
    buffer->ptr[buffer->end] = (char) ch;
    ++buffer->end;
    buffer->ptr[buffer->end] = '\0';
}

void
buffer_append_cstr(buffer_t *buffer, const char *cstr) {
    assert(cstr);
    int length = (int) strlen(cstr);
    buffer_append(buffer, cstr, length);
}

//
// -- mark
void
set_mark(buffer_t *buffer) {
    assert(buffer);
    buffer->mark = buffer->current;
}

int
get_mark(buffer_t *buffer) {
    assert(buffer);
    return buffer->mark;
}

//
// -- current character
int
get_ch(buffer_t *buffer) {
    return prev_ch(buffer, 0);
}

//
// -- nth previous character
int
prev_ch(buffer_t *buffer, int n) {
    assert(buffer);
    int current = buffer->current - n;
    if (current < buffer->begin ||
        current >= buffer->end) {
        return EOF;
    }
    return buffer->ptr[current];
}

//
// -- current
void
set_current_begin(buffer_t *buffer) {
    assert(buffer);
    buffer->current = buffer->begin;
}

void
set_current_last(buffer_t *buffer) {
    assert(buffer);
    buffer->current = buffer->end - 1;
    if (buffer->current < buffer->begin) {
        buffer->current = buffer->begin;
    }
}

void
advance(buffer_t *buffer) {
    assert(buffer);
    if (buffer->current < buffer->end) {
        ++buffer->current;
    }
}

void
backup(buffer_t *buffer) {
    assert(buffer);
    if (buffer->current > buffer->begin) {
        --buffer->current;
    }
}

//
// -- buffer editing
void
trim_front(buffer_t *buffer, int n) {
    assert(buffer);
    assert(n > 0);
    buffer->begin += n;
    if (buffer->begin > buffer->end) {
        buffer->begin = buffer->end;
    }
    if (buffer->current < buffer->begin) {
        buffer->current = buffer->begin;
    }
}

void
trim_back(buffer_t *buffer, int n) {
    assert(buffer);
    assert(n > 0);
    buffer->end -= n;
    buffer->current -= n;
    if (buffer->end < buffer->begin) {
        buffer->end = buffer->begin;
    }
    if (buffer->current < buffer->begin) {
        buffer->current = buffer->begin;
    }
    buffer->ptr[buffer->end] = '\0';
}

//
// -- input/output
int
buffer_getline(buffer_t *buffer, FILE *fp) {
    assert(buffer);
    buffer_reset(buffer);

    while (true) {
        int ch = getc(fp);
        int save_errno = errno;
        if (ferror(fp)) {
            errno = save_errno;
            return -1;
        }
        if (ch == EOF) {
            if (buffer->end != 0) break;
            return -1;
        }
        reserve(buffer, buffer->end + 2);
        buffer->ptr[buffer->end] = (char) ch;
        ++buffer->end;

        if (ch == '\n') break;
    }
    buffer->ptr[buffer->end] = '\0';
    return buffer->end - buffer->begin;
}

int
buffer_putline(buffer_t *buffer,
               int begin, int end, FILE *fp) {
    assert(buffer);
    assert(begin >= buffer->begin && end <= buffer->end);
    for (int i = begin; i < end; ++i) {
        int ch = putc(buffer->ptr[i], fp);
        if (ch == EOF) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    return end - begin;
}

const char *
get_str(buffer_t *buffer) {
    assert(buffer);
    reserve(buffer, 1);
    buffer->ptr[buffer->end] = '\0';
    return buffer->ptr + buffer->begin;
}

const char *
get_current_str(buffer_t *buffer) {
    assert(buffer);
    reserve(buffer, 1);
    buffer->ptr[buffer->end] = '\0';
    return buffer->ptr + buffer->current;
}
