//
// -- main.c
//
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "lookup.h"
#include "pmark.h"
#include "scanner.h"
#include "translate.h"
#include "warning.h"

//
// -- globals
FILE *in_fp;
FILE *out_fp;
int cflag;
int lflag;
int line_count;

//
// -- input line buffer
static buffer_t line;

static void
usage() {
    fprintf(stderr, "usage: xlate-8080 [-cl] [file]\n");
    fprintf(stderr, "  translates i8080 mnemonics to the equivalent Z80 mnemonics\n");
    fprintf(stderr, "  reads from file (or stdin if file not given on command line)\n");
    fprintf(stderr, "    and translates the source, writing to stdout\n");
    fprintf(stderr, "options:\n");
    fprintf(stderr, "  -c honor source file uppercase/lowercase usage\n");
    fprintf(stderr, "  -l translate z80.lib macros\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[]) {
    // -- process command line arguments
    int ch;
    while ((ch = getopt(argc, argv, "cl")) != -1) {
        switch (ch) {
            case 'c':
                cflag = 1;
                break;
            case 'l':
                lflag = 1;
                break;
            default:
                usage();
        }
    }
    argv += optind;

    // -- open input file
    char *path = argv[0];
    if (!path || strcmp(path, "-") == 0) {
        in_fp = stdin;
    } else {
        in_fp = fopen(path, "r");
        if (!in_fp) {
            perror("open");
            exit(EXIT_FAILURE);
        }
    }
    out_fp = stdout;

    // -- read the input line by line
    int read;
    line_count = 0;
    while ((read = buffer_getline(&line, in_fp)) != -1) {
        ++line_count;
        reset_pmark();
        // -- skip editor line numbers (for compatibility)
        skip_line_number(&line);
        // -- skip '*'-style comment lines (for compatibility)
        if (get_ch(&line) != '*') {
            do {
                skip_whitespace(&line);
                // -- done if end of line or comment
                if (is_statement_end(get_ch(&line))) break;

                entry_t *entry = scan_statement(&line);
                if (entry) {
                    do_subst(&line, entry);
                }
                skip_whitespace(&line);
                // -- handle multiple statements on line
            } while (match(&line, '!'));
        }
        // -- output the remainder of the input line
        buffer_putline(&line, get_pmark(), read, out_fp);
    }

    // -- report read error
    if (ferror(in_fp)) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // -- and exit - the system will close files and release memory
    exit(EXIT_SUCCESS);
}
