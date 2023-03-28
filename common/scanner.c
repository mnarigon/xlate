//
// -- scanner.c
//
#include "scanner.h"

#include <ctype.h>
#include <string.h>

#include "buffer.h"
#include "lookup.h"
#include "warning.h"

extern int lflag;

//
// -- true if the scanned identifier is lower case
// -- used for matching output case
bool is_lc_identifier;

static buffer_t identifier;
static buffer_t number;

bool
is_whitespace(int ch) {
    return ch == ' ' || ch == '\t';
}

bool
is_numeric(int ch) {
    return isdigit(ch);
}

bool
is_identifier_start(int ch) {
    return isalpha(ch) || ch == '?' || ch == '@';
}

bool
is_identifier_continuation(int ch) {
    return isalnum(ch) || ch == '?' || ch == '@' || ch == '$';
}

bool is_argument_end(int ch) {
    return (ch == ',' || ch == ';' || ch == '!'|| 
            ch == '\n' || ch == '\r' || ch == '\0');
}

bool is_statement_end(int ch) {
    return (ch == ';' ||
            ch == '\n' || ch == '\r' || ch == '\0');
}

void
skip_whitespace(buffer_t *line) {
    while (is_whitespace(get_ch(line))) {
        advance(line);
    }
}

void
skip_line_number(buffer_t *line) {
    while (is_numeric(get_ch(line))) {
        advance(line);
    }
}

bool
match(buffer_t *line, int ch) {
    if (get_ch(line) == ch) {
        advance(line);
        return true;
    }
    return false;
}

//
// -- scan for a number
// -- ignores '$' in numbers
// -- returns value
int
scan_number(const char *str) {
    buffer_reset(&number);
    unsigned int value = 0;
    unsigned int radix = 10;
    do {
        if (*str != '$') {
            buffer_append_ch(&number, toupper(*str));
        }
        ++str;
    } while (isxdigit(*str) || *str == '$');

    // -- test for radix tag
    if (toupper(*str) == 'O' || toupper(*str) == 'Q') {
        radix = 8;
        ++str;
    } else if (toupper(*str) == 'H') {
        radix = 16;
        ++str;
    } else {
        set_current_last(&number);
        // -- check last char in buffer
        int ch = get_ch(&number);
        if (ch == 'B') {
            radix = 2;
            trim_back(&number, 1);
        } else if (ch == 'D') {
            radix = 10;
            trim_back(&number, 1);
        }
    }

    // -- convert
    const char *ptr = get_str(&number);
    while (*ptr) {
        unsigned int digit = (*ptr >= 'A') ? (*ptr - 'A' + 10) : (*ptr - '0');
        ++ptr;
        unsigned int tmp = value * radix + digit;
        if (digit >= radix || tmp < value) {
            warning("can't scan number");
            return 0;
        }
        value = tmp;
    }
    return value;
}

static void
scan_identifier(buffer_t *line, buffer_t *identifier) {
    is_lc_identifier = false;
    buffer_reset(identifier);
    int ch = get_ch(line);
    is_lc_identifier = is_lc_identifier || islower(ch);

    buffer_append_ch(identifier, ch);
    advance(line);
    while (is_identifier_continuation(ch = get_ch(line))) {
        if (ch != '$') {
            is_lc_identifier = is_lc_identifier || islower(ch) != 0;
            buffer_append_ch(identifier, ch);
        }
        advance(line);
    }
    if (ch == ':') {
        buffer_append_ch(identifier, ch);
        advance(line);
    }
}

static void
scan_string(buffer_t *line, buffer_t *argument) {
    // -- save first single quote
    buffer_append_ch(argument, get_ch(line));
    advance(line);

    while (true) {
        int ch = get_ch(line);
        if (ch == '\0' || ch == '\n') { // -- unexpected end of line
            warning("unterminated string");
            break;
        }
        if (ch == '\'') { // -- test for doubled single quote
            buffer_append_ch(argument, ch);
            advance(line);
            ch = get_ch(line);
            if (ch != '\'') break; // -- single ending quote
        }
        buffer_append_ch(argument, ch);
        advance(line);
    }
}

void
scan_argument(buffer_t *line, buffer_t *argument) {
    buffer_reset(argument);
    while (true) {
        int ch = get_ch(line);
        if (is_argument_end(ch)) break;
        if (ch == '\'' &&
            !(toupper(prev_ch(line, 1)) == 'F' &&
              toupper(prev_ch(line, 2)) == 'A'))  {
            // -- scan a string literal except if it is AF'
            scan_string(line, argument);
        } else {
            buffer_append_ch(argument, ch);
            advance(line);
        }
    }
    // -- trim ending space
    set_current_last(argument);
    while (is_whitespace(get_ch(argument))) {
        trim_back(argument, 1);
    }
}

//
// -- scan a statement and look for opcodes
entry_t *
scan_statement(buffer_t *line) {
    entry_t *entry = NULL;
    // -- first identifier in statement, check if opcode
    if (is_identifier_start(get_ch(line))) {
        set_mark(line);
        scan_identifier(line, &identifier);
        entry = lookup_opcode(get_str(&identifier));
        if (lflag && !entry) {
            entry = lookup_libcode(get_str(&identifier));
        }
        if (!entry) {
            // -- not an opcode, check second identifier
            skip_whitespace(line);
            if (is_identifier_start(get_ch(line))) {
                set_mark(line);
                scan_identifier(line, &identifier);
                entry = lookup_opcode(get_str(&identifier));
                if (lflag && !entry) {
                    entry = lookup_libcode(get_str(&identifier));
                }
            }
        }
    }
    // -- return lookup table entry for opcode or
    // -- null if there was no opcode in statement
    return entry;
}

// -- return the displacement from an indexed argument - modifies argument
const char *
scan_displacement(struct buffer *argument) {
    static const char zero[] = "0";
    const char *disp = NULL;
    set_current_begin(argument);
    // -- test open paren
    if (!match(argument, '(')) {
        warning("can't scan displacement, missing open paren");
        return zero;
    }
    // -- optional whitespace
    skip_whitespace(argument);
    // -- test "IX"
    if (toupper(get_ch(argument)) != 'I') {
        warning("can't scan displacement, missing register");
        return zero;
    }
    advance(argument);
    int ch = toupper(get_ch(argument));
    if (ch != 'X' && ch != 'Y') {
        warning("can't scan displacement, missing register");
        return zero;
    }
    advance(argument);
    // -- optional whitespace
    skip_whitespace(argument);
    // -- skip '+'
    match(argument, '+');
    // -- optional whitespace
    skip_whitespace(argument);
    // -- start of displacement
    disp = get_current_str(argument);

    // -- move to last char
    set_current_last(argument);
    // -- test close paren
    if (get_ch(argument) != ')') {
        warning("can't scan displacement, missing close paren");
        return zero;
    }
    // -- remove close paren and whitespace
    trim_back(argument, 1);
    while (is_whitespace(get_ch(argument))) {
        trim_back(argument, 1);
    }
    // -- if empty displacement return zero
    if (strlen(disp) == 0) {
        return zero;
    }
    return disp;
}

//
// -- return the address from an indirect address argument - modifies argument
const char *
scan_indirect(struct buffer *argument) {
    const char *addr = NULL;

    set_current_begin(argument);
    // -- test open paren
    if (!match(argument, '(')) {
        warning("can't scan indirect address, missing open paren");
        return "";
    }
    // -- optional whitespace
    skip_whitespace(argument);
    // -- start of address
    addr = get_current_str(argument);

    // -- move to last char
    set_current_last(argument);
    // -- test close paren
    if (get_ch(argument) != ')') {
        warning("can't scan indirect address, missing close paren");
        return "";
    }
    // -- remove close paren and whitespace
    trim_back(argument, 1);
    while (is_whitespace(get_ch(argument))) {
        trim_back(argument, 1);
    }
    return addr;
}
