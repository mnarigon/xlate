//
// -- pmark.h
//
#ifndef PMARK_H
#define PMARK_H

//
// -- manage a mark of how far the line buffer has been printed

//
// -- reset pmark to zero (beginning of buffer)
void reset_pmark(void);

//
// -- set pmark to buffer current
void set_pmark(int mark);

//
// -- get the pmark
int get_pmark(void);

//
// -- set pmark to the last pmark (for error recovery)
void last_pmark(void);

#endif
