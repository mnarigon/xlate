//
// -- lookup.h
//
#ifndef LOOKUP_H
#define LOOKUP_H

typedef struct lookup {
    const char orig[8];  // -- the i8080 opcode to look up
    const char repl[8];  // -- the z80 replacement opcode
    const char arg0[8];  // -- code for how to generate z80 argument 0
    const char arg1[8];  // -- code for how to generate z80 argument 1
} entry_t;

//
// -- lookup an i8080 opcode entry
entry_t *lookup_opcode(const char *identifier);

//
// -- lookup a z80.lib macro entry
entry_t *lookup_libcode(const char *identifier);

//
// -- lookup register sets with z80 replacements
// -- register set 0 - B, C, D, E, H, L, A
// -- register set 1 - B, C, D, E, H, L, M, A
// -- register set 2 - B, D
// -- register set 3 - B, D, H, SP
// -- register set 4 - B, D, H, PSW
// -- register set 5 - BC, DE, HL, SP - z80.lib definition
// -- register set 6 - BC, DE, IX, SP - z80.lib definition
// -- register set 7 - BC, DE, IY, SP - z80.lib definition

const char *lookup_reg_set0(const char *arg);
const char *lookup_reg_set1(const char *arg);
const char *lookup_reg_set2(const char *arg);
const char *lookup_reg_set3(const char *arg);
const char *lookup_reg_set4(const char *arg);
const char *lookup_reg_set5(const char *arg);
const char *lookup_reg_set6(const char *arg);
const char *lookup_reg_set7(const char *arg);

#endif
