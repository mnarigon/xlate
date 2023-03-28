//
// -- buffer.h
//
#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>

typedef struct buffer {
    int begin;
    int end;
    int capacity;
    char *ptr;
    int current;
    int mark;
} buffer_t;

//
// -- initialize the buffer
buffer_t *buffer_init(buffer_t *buffer);

//
// -- free allocated memory
void buffer_free(buffer_t *buffer);

//
// -- reset contents
void buffer_reset(buffer_t *buffer);

//
// -- buffer content size
int buffer_size(buffer_t *buffer);

//
// -- append to the buffer
void buffer_append(buffer_t *buffer, const char *ptr, int length);
void buffer_append_ch(buffer_t *buffer, int ch);
void buffer_append_cstr(buffer_t *buffer, const char *cstr);

//
// -- mark
void set_mark(buffer_t *buffer);
int get_mark(buffer_t *buffer);

//
// -- current character
int get_ch(buffer_t *buffer);

//
// -- nth previous character
int prev_ch(buffer_t *buffer, int n);

//
// -- current
void set_current_begin(buffer_t *buffer);
void set_current_last(buffer_t *buffer);
void advance(buffer_t *buffer);
void backup(buffer_t *buffer);

//
// -- buffer editing
void trim_front(buffer_t *buffer, int n);
void trim_back(buffer_t *buffer, int n);

//
// -- input/output
int buffer_getline(buffer_t *buffer, FILE *fp);
int buffer_putline(buffer_t *buffer,
                   int begin, int end, FILE *fp);
const char *get_str(buffer_t *buffer);
const char *get_current_str(buffer_t *buffer);

#endif
