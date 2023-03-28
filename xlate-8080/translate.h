//
// -- translate.h
//
#ifndef TRANSLATE_H
#define TRANSLATE_H

// -- forward declaration
struct buffer;
struct lookup;

//
// substitute opcodes according to the lookup table entry
void do_subst(struct buffer *line, struct lookup *entry);

#endif
