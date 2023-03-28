//
// -- translate.c
//
#include "translate.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"
#include "lookup.h"
#include "pmark.h"
#include "scanner.h"

extern FILE *out_fp;
extern int cflag;

static buffer_t argument0;
static buffer_t argument1;

//
// -- output string according to the case of the identifier
static void
putcasestr(const char *str) {
    if (cflag && is_lc_identifier) {
        while (*str) {
            fputc(tolower(*str++), out_fp);
        }
    } else {
        while (*str) {
            fputc(toupper(*str++), out_fp);
        }
    }
}

//
// -- output string
static void
putstr(const char *str) {
    fprintf(out_fp, "%s", str);
}

//
// -- based on the argument code, select the appropriate argument buffer
static const char *
select_arg(int ch) {
    if (ch == '0') {
        return get_str(&argument0);
    }
    if (ch == '1') {
        return get_str(&argument1);
    }
    return NULL;
}

//
// -- argument code
// -- X - literal
// -- $An - nth argument
// -- $Bn - nth argument, in parenthesis
// -- $Cn - register set 0 - B, C, D, E, H, L, A
// -- $Dn - register set 1 - B, C, D, E, H, L, M, A ; M -> (HL))
// -- $En - register set 2 - B, D                   ; B -> (BC), D -> (DE)
// -- $Fn - register set 3 - B, D, H, SP            ; B -> BC, D -> DE, H -> HL, SP -> SP
// -- $Gn - register set 4 - B, D, H, PSW           ; B -> BC, D -> DE, H -> HL, PSW -> AF
// -- $Hn - argument is number, multiply by 8 and output as hexadecimal
// -- $In - nth argument is displacement from IX
// -- $Jn - nth argument is displacement from IY
// -- $Kn - register set 5 - BC, DE, HL, SP
// -- $Ln - register set 6 - BC, DE, IX, SP
// -- $Mn - register set 7 - BC, DE, IY, SP

//
// -- generate an argument according to the argument code
void
do_argument(const char *arg) {
    if (arg[0] != '$') {
        putcasestr(arg);
        return;
    }
    switch(arg[1]) {
        case 'A':
            putstr(select_arg(arg[2]));
            break;
        case 'B':
            putstr("(");
            putstr(select_arg(arg[2]));
            putstr(")");
            break;
        case 'C':
            putcasestr(lookup_reg_set0(select_arg(arg[2])));
            break;
        case 'D':
            putcasestr(lookup_reg_set1(select_arg(arg[2])));
            break;
        case 'E':
            putcasestr(lookup_reg_set2(select_arg(arg[2])));
            break;
        case 'F':
            putcasestr(lookup_reg_set3(select_arg(arg[2])));
            break;
        case 'G':
            putcasestr(lookup_reg_set4(select_arg(arg[2])));
            break;
        case 'H': {
            // -- handle the RST offset
            int value = scan_number(select_arg(arg[2])) * 8;
            if (value < 10) {
                fprintf(out_fp, "%1XH", value);
            } else if (value < 16) {
                fprintf(out_fp, "0%1XH", value);
            } else {
                fprintf(out_fp, "%2XH", value);
            }
            break;
        }
        case 'I': {
            const char *str = select_arg(arg[2]);
            putcasestr("(IX");
            // -- handle the displacement
            // -- ignore 0, add plus only if there isn't already a sign
            if (str[0] != '0' || strlen(str) != 1) {
                if (str[0] != '+' && str[0] != '-') {
                    putstr("+");
                }
                putstr(str);
            }
            putstr(")");
            break;
        }
        case 'J': {
            const char *str = select_arg(arg[2]);
            putcasestr("(IY");
            // -- handle the displacement
            // -- ignore 0, add plus only if there isn't already a sign
            if (str[0] != '0' || strlen(str) != 1) {
                if (str[0] != '+' && str[0] != '-') {
                    putstr("+");
                }
                putstr(str);
            }
            putstr(")");
            break;
        }
        case 'K':
            putcasestr(lookup_reg_set5(select_arg(arg[2])));
            break;
        case 'L':
            putcasestr(lookup_reg_set6(select_arg(arg[2])));
            break;
        case 'M':
            putcasestr(lookup_reg_set7(select_arg(arg[2])));
            break;
    }
}

static void
prep_arguments(buffer_t *line, entry_t *entry) {
    // -- output the input line up to the mark
    buffer_putline(line, get_pmark(), get_mark(line), stdout);
    // -- scan line for the arguments
    buffer_reset(&argument0);
    buffer_reset(&argument1);

    skip_whitespace(line);
    scan_argument(line, &argument0);

    skip_whitespace(line);
    if (match(line, ',')) {
        skip_whitespace(line);
        scan_argument(line, &argument1);
    }

    // -- backup over ending whitespace
    while (is_whitespace(prev_ch(line, 1))) {
        backup(line);
    }

    // -- adjust the print mark to the end of the arguments
    set_pmark(line->current);
}

//
// substitute opcodes according to the lookup table entry
void
do_subst(buffer_t *line, entry_t *entry) {
    prep_arguments(line, entry);

    // -- output the replacement opcode
    putcasestr(entry->repl);
    // -- and process the arguments
    if (entry->arg0[0]) {
        putstr("\t");
        do_argument(entry->arg0);
    }
    if (entry->arg1[0]) {
        putstr(",");
        do_argument(entry->arg1);
    }
}
