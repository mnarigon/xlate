//
// -- lookup.c
//
#include "lookup.h"

#include <ctype.h>
#include <string.h>

#include "scanner.h"
#include "translate.h"
#include "warning.h"

//
// -- Intel 8080 mnemonics from:
// -- "8080/8085 Assembly Language Programming Manual," 1981, Intel Corporation
// -- Intel mnemonics Copyright (c) 1974, 1975, 1976, 1977 Intel Corporation

// -- Zilog Z80 mnemonics from:
// -- "Z80 Assembly Language Programming Manual," 1977, Zilog, Inc.

typedef struct {
    const char orig[8];
    const char repl[8];
} set_t;

static set_t set1_table[] = {
    { "A",    "A" },
    { "B",    "B" },
    { "C",    "C" },
    { "D",    "D" },
    { "E",    "E" },
    { "H",    "H" },
    { "L",    "L" },
    { "(HL)", "M" }
};
static const int set1_size = (int) (sizeof(set1_table) / sizeof(set_t));

static set_t set2_table[] = {
    { "(BC)", "B" },
    { "(DE)", "D" }
};
static const int set2_size = (int) (sizeof(set2_table) / sizeof(set_t));

static set_t set3_table[] = {
    { "BC",   "B" },
    { "DE",   "D" },
    { "HL",   "H" },
    { "SP",   "SP" }
};
static const int set3_size = (int) (sizeof(set3_table) / sizeof(set_t));

static set_t set4_table[] = {
    { "BC",   "B" },
    { "DE",   "D" },
    { "HL",   "H" },
    { "AF",   "PSW" }
};
static const int set4_size = (int) (sizeof(set4_table) / sizeof(set_t));

static const char *
lookup_set(const char *arg, set_t *table, int size) {
    for (int i = 0; i < size; ++i) {
        if (strcasecmp(arg, table[i].orig) == 0) {
            return table[i].repl;
        }
    }
    warning("invalid register");
    return arg;
}

const char *
lookup_set1(const char *arg) {
    return lookup_set(arg, set1_table, set1_size);
}

const char *
lookup_set2(const char *arg) {
    return lookup_set(arg, set2_table, set2_size);
}

const char *
lookup_set3(const char *arg) {
    return lookup_set(arg, set3_table, set3_size);
}

const char *
lookup_set4(const char *arg) {
    return lookup_set(arg, set4_table, set4_size);
}


//
// -- argument code
// -- X - index into name tables
// -- $An - nth argument
// -- $Hn - argument is hex number, divide by 8 and output as decimal
static entry_t opcode_table[] = {
    { do_adc_sbc, "ADC",   "",      "0" },
    { do_add,     "ADD",   "",      0 },
    { do_operi,   "AND",   "",      "0" },
    { do_bits,    "BIT",   "",      "0" },
    { do_call,    "CALL",  "",      0 },
    { do_subst,   "CCF",   "CMC",   0 },
    { do_operi,   "CP",    "",      "1" },
    { do_subst,   "CPD",   "CCD",   0 },
    { do_subst,   "CPDR",  "CCDR",  0 },
    { do_subst,   "CPI",   "CCI",   0 },
    { do_subst,   "CPIR",  "CCIR",  0 },
    { do_subst,   "CPL",   "CMA",   0 },
    { do_subst,   "DAA",   "DAA",   0 },
    { do_dec_inc, "DEC",   "",      "0" },
    { do_subst,   "DI",    "DI",    0 },
    { do_subst,   "DJNZ",  "DJNZ",  "$A0" },
    { do_subst,   "EI",    "EI",    0 },
    { do_ex,      "EX",    "",      0 },
    { do_subst,   "EXX",   "EXX",   0 },
    { do_subst,   "HALT",  "HLT",   0 },
    { do_im,      "IM",    "",      0 },
    { do_in,      "IN",    "",      0 },
    { do_dec_inc, "INC",   "",      "1" },
    { do_subst,   "IND",   "IND",   0 },
    { do_subst,   "INDR",  "INDR",  0 },
    { do_subst,   "INI",   "INI",   0 },
    { do_subst,   "INIR",  "INIR",  0 },
    { do_jp,      "JP",    "",      0 },
    { do_jr,      "JR",    "",      0 },
    { do_ld,      "LD",    "",      0 },
    { do_subst,   "LDD",   "LDD",   0 },
    { do_subst,   "LDDR",  "LDDR",  0 },
    { do_subst,   "LDI",   "LDI",   0 },
    { do_subst,   "LDIR",  "LDIR",  0 },
    { do_subst,   "NEG",   "NEG",   0 },
    { do_subst,   "NOP",   "NOP",   0 },
    { do_operi,   "OR",    "",      "2" },
    { do_subst,   "OTDR",  "OUTDR", 0 },
    { do_subst,   "OTIR",  "OUTIR", 0 },
    { do_out,     "OUT",   "",      0 },
    { do_subst,   "OUTD",  "OUTD",  0 },
    { do_subst,   "OUTI",  "OUTI",  0 },
    { do_pop_psh, "POP",   "POP",   0 },
    { do_pop_psh, "PUSH",  "PUSH",  0 },
    { do_bits,    "RES",   "",      "1" },
    { do_ret,     "RET",   "",      0 },
    { do_subst,   "RETI",  "RETI",  0 },
    { do_subst,   "RETN",  "RETN",  0 },
    { do_opers,   "RL",    "",      "0" },
    { do_subst,   "RLA",   "RAL",   0 },
    { do_opers,   "RLC",   "",      "1" },
    { do_subst,   "RLCA",  "RLC",   0 },
    { do_subst,   "RLD",   "RLD",   0 },
    { do_opers,   "RR",    "",      "2" },
    { do_subst,   "RRA",   "RAR",   0 },
    { do_opers,   "RRC",   "",      "3" },
    { do_subst,   "RRCA",  "RRC",   0 },
    { do_subst,   "RRD",   "RRD",   0 },
    { do_subst,   "RST",   "RST",   "$H0" },
    { do_adc_sbc, "SBC",   "",      "1" },
    { do_subst,   "SCF",   "STC",   0 },
    { do_bits,    "SET",   "",      "2" },
    { do_opers,   "SLA",   "",      "4" },
    { do_opers,   "SRA",   "",      "5" },
    { do_opers,   "SRL",   "",      "6" },
    { do_operi,   "SUB",   "",      "3" },
    { do_operi,   "XOR",   "",      "4" },
};
static const int opcode_size = (int) (sizeof(opcode_table) / sizeof(entry_t));

entry_t *
lookup_opcode(const char *arg) {
    // -- binary search
    int l = 0;
    int r = opcode_size;
    while (l < r) {
        int m = (r - l) / 2 + l;
        int cmp = strcasecmp(arg, opcode_table[m].orig);
        if (cmp < 0) {
            r = m;
        } else if (cmp > 0) {
            l = m + 1;
        } else {
            return &opcode_table[m];
        }
    }
    return NULL;
}

entry_t *
lookup_libcode(const char *arg) {
    return NULL;
}

// -- register set 0 - B, C, D, E, H, L, A
// -- register set 1 - B, C, D, E, H, L, (HL), A
// -- register set 2 - (BC), (DE)
// -- register set 3 - BC, DE, HL, SP
// -- register set 4 - BC, DE, HL, AF
// -- register set 5 - BC, DE, HL, SP
// -- register set 6 - BC, DE, IX, SP
// -- register set 7 - BC, DE, IY, SP

// -- code defines
#define IS_SET0 0X20000000
#define IS_SET1 0X10000000
#define IS_SET2 0X08000000
#define IS_SET3 0X04000000
#define IS_SET4 0X02000000
#define IS_SET5 0X01000000
#define IS_SET6 0X00800000
#define IS_SET7 0X00400000

#define IS_A    0X00200000
#define IS_I    0X00100000
#define IS_R    0X00080000

#define IS_AF   0X00040000
#define IS_AFP  0X00020000
#define IS_BC   0X00010000
#define IS_DE   0X00008000
#define IS_HL   0X00004000
#define IS_SP   0X00002000
#define IS_IX   0X00001000
#define IS_IY   0X00000800

#define IS_CI   0X00000400
#define IS_BCI  0X00000200
#define IS_DEI  0X00000100
#define IS_HLI  0X00000080
#define IS_SPI  0X00000040
#define IS_IXI  0X00000020
#define IS_IYI  0X00000010

#define IS_IXD  0X00000008
#define IS_IYD  0X00000004

#define IS_IND  0X00000002
#define IS_IMM  0X00000001

typedef struct {
    const char orig[8];
    int code;
} reg_t;

static reg_t reg_table[] = {
    { "(BC)", IS_SET2 | IS_BCI | IS_IND },
    { "(C)",  IS_CI | IS_IND },
    { "(DE)", IS_SET2 | IS_DEI | IS_IND },
    { "(HL)", IS_SET1 | IS_HLI | IS_IND },
    { "(IX)", IS_IXI | IS_IND },
    { "(IY)", IS_IYI | IS_IND },
    { "(SP)", IS_SPI | IS_IND },
    { "A",    IS_SET0 | IS_SET1 | IS_A },
    { "AF",   IS_SET4 | IS_AF },
    { "AF'",  IS_AFP },
    { "B",    IS_SET0 | IS_SET1 },
    { "BC",   IS_SET3 | IS_SET4 | IS_SET5 | IS_SET6 | IS_SET7 | IS_BC },
    { "C",    IS_SET0 | IS_SET1 },
    { "D",    IS_SET0 | IS_SET1 },
    { "DE",   IS_SET3 | IS_SET4 | IS_SET5 | IS_SET6 | IS_SET7 | IS_DE },
    { "E",    IS_SET0 | IS_SET1 },
    { "H",    IS_SET0 | IS_SET1 },
    { "HL",   IS_SET3 | IS_SET4 | IS_SET5 | IS_HL },
    { "I",    IS_I },
    { "IX",   IS_SET6 | IS_IX },
    { "IY",   IS_SET7 | IS_IY },
    { "L",    IS_SET0 | IS_SET1 },
    { "R",    IS_R },
    { "SP",   IS_SET3 | IS_SET5 | IS_SET6 | IS_SET7 | IS_SP },
};
static const int reg_size = (int) (sizeof(reg_table) / sizeof(reg_t));

//
// -- test for indirect address
static bool
test_indirect(const char *arg) {
    if (*arg != '(') return false;
    if (*(arg + strlen(arg) - 1) != ')') return false;
    return true;
}

//
// -- test for indexed reg
static bool
test_indexed(const char *reg, const char *arg) {
    // -- test for beginning paren
    if (*arg != '(') return false;
    // -- test for ending paren
    if (*(arg + strlen(arg) - 1) != ')') return false;
    // -- advance past beginning paren
    ++arg;

    // -- skip whitespace
    while (is_whitespace(*arg)) {
        ++arg;
    }
    // -- test for register
    if (toupper(*arg) != reg[0]) return false;
    ++arg;
    if (toupper(*arg) != reg[1]) return false;
    ++arg;

    // -- test for identifier continuation character
    if (is_identifier_continuation(*arg)) return false;
    return true;
}

static int
lookup_reg_code(const char *arg) {
    // -- binary search
    int l = 0;
    int r = reg_size;
    while (l < r) {
        int m = (r - l) / 2 + l;
        int cmp = strcasecmp(arg, reg_table[m].orig);
        if (cmp < 0) {
            r = m;
        } else if (cmp > 0) {
            l = m + 1;
        } else {
            return reg_table[m].code;
        }
    }
    return 0;
}

//
// -- lookup an argument, returning code
int
lookup_arg_code(const char *arg) {
    // -- test for empty arg
    if (arg[0] == '\0') return 0;
    // -- lookup register
    int code = lookup_reg_code(arg);
    if (code != 0) return code;
    // -- test for indexed registers
    if (test_indexed("IX", arg)) return IS_IXD;
    if (test_indexed("IY", arg)) return IS_IYD;
    // -- test if indirect address
    if (test_indirect(arg)) return IS_IND;
    // -- is an immediate
    return IS_IMM;
}

//
// -- argument tests
bool
is_set0(int code) {
    return code & IS_SET0;
}

bool
is_set1(int code) {
    return code & IS_SET1;
}

bool
is_set2(int code) {
    return code & IS_SET2;
}

bool
is_set3(int code) {
    return code & IS_SET3;
}

bool
is_set4(int code) {
    return code & IS_SET4;
}

bool
is_set5(int code) {
    return code & IS_SET5;
}

bool
is_set6(int code) {
    return code & IS_SET6;
}

bool
is_set7(int code) {
    return code & IS_SET7;
}

bool
isA(int code) {
    return code & IS_A;
}

bool
isCi(int code) {
    return code & IS_CI;
}

bool
isI(int code) {
    return code & IS_I;
}

bool
isR(int code) {
    return code & IS_R;
}

bool
isAF(int code) {
    return code & IS_AF;
}

bool
isAFp(int code) {
    return code & IS_AFP;
}

bool
isBC(int code) {
    return code & IS_BC;
}

bool
isDE(int code) {
    return code & IS_DE;
}

bool
isHL(int code) {
    return code & IS_HL;
}

bool
isSP(int code) {
    return code & IS_SP;
}

bool
isIX(int code) {
    return code & IS_IX;
}

bool
isIY(int code) {
    return code & IS_IY;
}

bool
isHLi(int code) {
    return code & IS_HLI;
}

bool
isSPi(int code) {
    return code & IS_SPI;
}

bool
isIXi(int code) {
    return code & IS_IXI;
}

bool
isIYi(int code) {
    return code & IS_IYI;
}

bool
is_IX_indexed(int code) {
    return code & IS_IXD;
}

bool
is_IY_indexed(int code) {
    return code & IS_IYD;
}

bool
is_indirect(int code) {
    return code & IS_IND;
}

bool
is_immediate(int code) {
    return code & IS_IMM;
}

bool
is_null(int code) {
    return code == 0;
}

static const char *cc[] = {
    "C", "M", "NC", "NZ", "P", "PE", "PO", "Z"
};
static const int cc_size = (int) (sizeof(cc) / sizeof(const char *));

static const char *jr_cc[] = {
    "C", "NC", "NZ", "Z"
};
static const int jr_cc_size = (int) (sizeof(jr_cc) / sizeof(const char *));

bool
is_cc(const char *arg) {
    for (int i = 0; i < cc_size; ++i) {
        if (strcasecmp(arg, cc[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool
is_jr_cc(const char *arg) {
    for (int i = 0; i < jr_cc_size; ++i) {
        if (strcasecmp(arg, jr_cc[i]) == 0) {
            return true;
        }
    }
    return false;
}
