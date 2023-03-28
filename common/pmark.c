//
// -- pmark.c
//
#include "pmark.h"

static int lmark;
static int pmark;

//
// -- manage a mark of how far the line buffer has been printed

//
// -- reset pmark to zero (beginning of buffer)
void
reset_pmark(void) {
    lmark = 0;
    pmark = 0;
}

//
// -- set pmark to buffer current
void
set_pmark(int mark) {
    lmark = pmark;
    pmark = mark;
}

//
// -- get the pmark
int
get_pmark(void) {
    return pmark;
}

//
// -- set pmark to the last pmark (for error recovery)
void
last_pmark(void) {
    pmark = lmark;
}

