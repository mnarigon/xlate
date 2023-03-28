//
// -- scanner.h
//
#ifndef SCANNER_H
#define SCANNER_H

#include <stdbool.h>

// -- forward declaration
struct buffer;

//
// -- true if the scanned identifier has lower case characters
// -- used for matching output case
extern bool is_lc_identifier;

//
// -- character classification functions
bool is_whitespace(int ch);
bool is_numeric(int ch);
bool is_identifier_start(int ch);
bool is_identifier_continuation(int ch);
bool is_argument_end(int ch);
bool is_statement_end(int ch);

//
// -- return true and advance if current matches ch
bool match(struct buffer *line, int ch);

//
// -- scan for a number
// -- ignores '$' in numbers
// -- returns value
int scan_number(const char *str);

//
// -- skip whitespace
void skip_whitespace(struct buffer *line);

//
// -- skip line number
void skip_line_number(struct buffer *line);

//
// -- scan line for an argument and place in argument
void scan_argument(struct buffer *line, struct buffer *argument);

//
// -- scan a statement and look for opcodes
struct lookup *scan_statement(struct buffer *line);

// -- return the displacement from an indexed argument - modifies argument
const char *scan_displacement(struct buffer *argument);

//
// -- return the address from an indirect address argument - modifies argument
const char *scan_indirect(struct buffer *argument);

#endif
