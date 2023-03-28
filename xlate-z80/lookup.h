//
// -- lookup.h
//
#ifndef LOOKUP_H
#define LOOKUP_H

#include <stdbool.h>

struct buffer;
struct lookup;
typedef void (function_t)(struct buffer *line, struct lookup *entry);

typedef struct lookup {
    function_t *handler; // -- process the opcode
    const char orig[8];  // -- the opcode to look up
    const char repl[8];  // -- the i8080 replacement opcode
    const char arg0[8];  // -- code for how to generate i8080 argument 0
} entry_t;

// -- register set 1 - B, C, D, E, H, L, (HL), A ; (HL) -> M
// -- register set 2 - (BC), (DE)                ; (BC) -> B, (DE) -> D
// -- register set 3 - BC, DE, HL, SP            ; BC -> B, DE -> D, HL -> H
// -- register set 4 - BC, DE, HL, AF            ; BC -> B, DE -> D, HL -> H, AF -> PSW
const char *lookup_set1(const char *arg);
const char *lookup_set2(const char *arg);
const char *lookup_set3(const char *arg);
const char *lookup_set4(const char *arg);

//
// -- lookup a Z80 opcode entry
entry_t *lookup_opcode(const char *identifier);

// -- N/A on Z80
entry_t *lookup_libcode(const char *arg);

//
// -- lookup an argument type code
int lookup_arg_code(const char *arg);

//
// -- argument tests
bool is_set0(int code);
bool is_set1(int code);
bool is_set2(int code);
bool is_set3(int code);
bool is_set4(int code);
bool is_set5(int code);
bool is_set6(int code);
bool is_set7(int code);

bool isA(int code);
bool isCi(int code);
bool isI(int code);
bool isR(int code);

bool isAF(int code);
bool isAFp(int code);
bool isBC(int code);
bool isDE(int code);
bool isHL(int code);
bool isSP(int code);
bool isIX(int code);
bool isIY(int code);
bool isHLi(int code);
bool isSPi(int code);
bool isIXi(int code);
bool isIYi(int code);

bool is_IX_indexed(int code);
bool is_IY_indexed(int code);
bool is_indirect(int code);
bool is_immediate(int code);
bool is_null(int code);

//
// -- condition code tests
bool is_cc(const char *arg);
bool is_jr_cc(const char *arg);

#endif
