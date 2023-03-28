//
// -- lookup.c
//
#include "lookup.h"

#include <string.h>

#include "warning.h"

//
// -- Intel 8080 mnemonics from:
// -- "8080/8085 Assembly Language Programming Manual," 1981, Intel Corporation
// -- Intel mnemonics Copyright (c) 1974, 1975, 1976, 1977 Intel Corporation

// -- Zilog Z80 mnemonics from:
// -- "Z80 Assembly Language Programming Manual," 1977, Zilog, Inc.

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

static entry_t opcode_table[] = {
    { "ACI",    "ADC",  "A", "$A0" },
    { "ADC",    "ADC",  "A", "$D0" },
    { "ADD",    "ADD",  "A", "$D0" },
    { "ADI",    "ADD",  "A", "$A0" },
    { "ANA",    "AND",  "$D0", 0 },
    { "ANI",    "AND",  "$A0", 0 },
    { "CALL",   "CALL", "$A0", 0 },
    { "CC",     "CALL", "C", "$A0" },
    { "CM",     "CALL", "M", "$A0" },
    { "CMA",    "CPL",  0, 0 },
    { "CMC",    "CCF",  0, 0 },
    { "CMP",    "CP",   "$D0", 0 },
    { "CNC",    "CALL", "NC", "$A0" },
    { "CNZ",    "CALL", "NZ", "$A0" },
    { "CP",     "CALL", "P", "$A0" },
    { "CPE",    "CALL", "PE", "$A0" },
    { "CPI",    "CP",   "$A0", 0 },
    { "CPO",    "CALL", "PO", "$A0" },
    { "CZ",     "CALL", "Z", "$A0" },
    { "DAA",    "DAA",  0, 0 },
    { "DAD",    "ADD",  "HL", "$F0" },
    { "DCR",    "DEC",  "$D0", 0 },
    { "DCX",    "DEC",  "$F0", 0 },
    { "DI",     "DI",   0, 0 },
    { "EI",     "EI",   0, 0 },
    { "HLT",    "HALT", 0, 0 },
    { "IN",     "IN",   "A", "$B0" },
    { "INR",    "INC",  "$D0", 0 },
    { "INX",    "INC",  "$F0", 0 },
    { "JC",     "JP",   "C", "$A0" },
    { "JM",     "JP",   "M", "$A0" },
    { "JMP",    "JP",   "$A0", 0 },
    { "JNC",    "JP",   "NC", "$A0" },
    { "JNZ",    "JP",   "NZ", "$A0" },
    { "JP",     "JP",   "P", "$A0" },
    { "JPE",    "JP",   "PE", "$A0" },
    { "JPO",    "JP",   "PO", "$A0" },
    { "JZ",     "JP",   "Z", "$A0" },
    { "LDA",    "LD",   "A", "$B0" },
    { "LDAX",   "LD",   "A", "$E0" },
    { "LHLD",   "LD",   "HL", "$B0" },
    { "LXI",    "LD",   "$F0", "$A1" },
    { "MOV",    "LD",   "$D0", "$D1" },
    { "MVI",    "LD",   "$D0", "$A1" },
    { "NOP",    "NOP",  0, 0 },
    { "ORA",    "OR",   "$D0", 0 },
    { "ORI",    "OR",   "$A0",  0 },
    { "OUT",    "OUT",  "$B0", "A" },
    { "PCHL",   "JP",   "(HL)", 0 },
    { "POP",    "POP",  "$G0", 0 },
    { "PUSH",   "PUSH", "$G0", 0 },
    { "RAL",    "RLA",  0, 0 },
    { "RAR",    "RRA",  0, 0 },
    { "RC",     "RET",  "C", 0 },
    { "RET",    "RET",  0, 0 },
    { "RLC",    "RLCA", 0, 0 },
    { "RM",     "RET",  "M", 0 },
    { "RNC",    "RET",  "NC", 0 },
    { "RNZ",    "RET",  "NZ", 0 },
    { "RP",     "RET",  "P", 0 },
    { "RPE",    "RET",  "PE", 0 },
    { "RPO",    "RET",  "PO", 0 },
    { "RRC",    "RRCA", 0, 0 },
    { "RST",    "RST",  "$H0", 0 },
    { "RZ",     "RET",  "Z", 0 },
    { "SBB",    "SBC",  "A", "$D0" },
    { "SBI",    "SBC",  "A", "$A0" },
    { "SHLD",   "LD",   "$B0", "HL" },
    { "SPHL",   "LD",   "SP", "HL" },
    { "STA",    "LD",   "$B0", "A" },
    { "STAX",   "LD",   "$E0", "A" },
    { "STC",    "SCF",  0, 0 },
    { "SUB",    "SUB",  "$D0", 0 },
    { "SUI",    "SUB",  "$A0", 0 },
    { "XCHG",   "EX",   "DE", "HL" },
    { "XRA",    "XOR",  "$D0", 0 },
    { "XRI",    "XOR",  "$A0", 0 },
    { "XTHL",   "EX",   "(SP)", "HL" }
};
static const int opcode_size = (int) (sizeof(opcode_table) / sizeof(entry_t));

//
// -- macros defined in z80.lib to represent z80-specific instructions
static entry_t lib_table[] = {
    { "ADCX",   "ADC",  "A", "$I0" },
    { "ADCY",   "ADC",  "A", "$J0" },
    { "ADDX",   "ADD",  "A", "$I0" },
    { "ADDY",   "ADD",  "A", "$J0" },
    { "ANDX",   "AND",  "$I0", 0 },
    { "ANDY",   "AND",  "$J0", 0 },
    { "BIT",    "BIT",  "$A0", "$D1" },
    { "BITX",   "BIT",  "$A0", "$I1" },
    { "BITY",   "BIT",  "$A0", "$J1" },
    { "CCD",    "CPD",  0, 0 },
    { "CCDR",   "CPDR", 0, 0 },
    { "CCI",    "CPI",  0, 0 },
    { "CCIR",   "CPIR", 0, 0 },
    { "CMPX",   "CP",   "$I0", 0 },
    { "CMPY",   "CP",   "$J0", 0 },
    { "DADC",   "ADC",  "HL", "$K0" },
    { "DADX",   "ADD",  "IX", "$L0" },
    { "DADY",   "ADD",  "IY", "$M0" },
    { "DCRX",   "DEC",  "$I0", 0 },
    { "DCRY",   "DEC",  "$J0", 0 },
    { "DCXIX",  "DEC",  "IX", 0 },
    { "DCXIY",  "DEC",  "IY", 0 },
    { "DJNZ",   "DJNZ", "$A0", 0 },
    { "DSBC",   "SBC",  "HL", "$K0" },
    { "EXAF",   "EX",   "AF", "AF'" },
    { "EXX",    "EXX",  0, 0 },
    { "IM0",    "IM",   "0", 0 },
    { "IM1",    "IM",   "1", 0 },
    { "IM2",    "IM",   "2", 0 },
    { "IND",    "IND",  0, 0 },
    { "INDR",   "INDR", 0, 0 },
    { "INI",    "INI",  0, 0 },
    { "INIR",   "INIR", 0, 0 },
    { "INP",    "IN",   "$C0", "(C)" },
    { "INRX",   "INC",  "$I0", 0 },
    { "INRY",   "INC",  "$J0", 0 },
    { "INXIX",  "INC",  "IX", 0 },
    { "INXIY",  "INC",  "IY", 0 },
    { "JR",     "JR",   "$A0", 0 },
    { "JRC",    "JR",   "C", "$A0" },
    { "JRNC",   "JR",   "NC", "$A0" },
    { "JRNZ",   "JR",   "NZ", "$A0" },
    { "JRZ",    "JR",   "Z", "$A0" },
    { "LBCD",   "LD",   "BC", "$B0" },
    { "LDAI",   "LD",   "A", "I" },
    { "LDAR",   "LD",   "A", "R" },
    { "LDD",    "LDD",  0, 0 },
    { "LDDR",   "LDDR", 0, 0 },
    { "LDED",   "LD",   "DE", "$B0" },
    { "LDI",    "LDI",  0, 0 },
    { "LDIR",   "LDIR", 0, 0 },
    { "LDX",    "LD",   "$C0", "$I1" },
    { "LDY",    "LD",   "$C0", "$J1" },
    { "LIXD",   "LD",   "IX", "$B0" },
    { "LIYD",   "LD",   "IY", "$B0" },
    { "LSPD",   "LD",   "SP", "$B0" },
    { "LXIX",   "LD",   "IX", "$A0" },
    { "LXIY",   "LD",   "IY", "$A0" },
    { "MVIX",   "LD",   "$I1", "$A0" },
    { "MVIY",   "LD",   "$J1", "$A0" },
    { "NEG",    "NEG",  0, 0 },
    { "ORX",    "OR",   "$I0", 0 },
    { "ORY",    "OR",   "$J0", 0 },
    { "OUTD",   "OUTD", 0, 0 },
    { "OUTDR",  "OTDR", 0, 0 },
    { "OUTI",   "OUTI", 0, 0 },
    { "OUTIR",  "OTIR", 0, 0 },
    { "OUTP",   "OUT",  "(C)", "$C0" },
    { "PCIX",   "JP",   "(IX)", 0 },
    { "PCIY",   "JP",   "(IY)", 0 },
    { "POPIX",  "POP",  "IX", 0 },
    { "POPIY",  "POP",  "IY", 0 },
    { "PUSHIX", "PUSH", "IX", 0 },
    { "PUSHIY", "PUSH", "IY", 0 },
    { "RALR",   "RL",   "$C0", 0 },
    { "RALX",   "RL",   "$I0", 0 },
    { "RALY",   "RL",   "$J0", 0 },
    { "RARR",   "RR",   "$C0", 0 },
    { "RARX",   "RR",   "$I0", 0 },
    { "RARY",   "RR",   "$J0", 0 },
    { "RES",    "RES",  "$A0", "$D1" },
    { "RESX",   "RES",  "$A0", "$I1" },
    { "RESY",   "RES",  "$A0", "$J1" },
    { "RETI",   "RETI", 0, 0 },
    { "RETN",   "RETN", 0, 0 },
    { "RLCR",   "RLC",  "$C0", 0 },
    { "RLCX",   "RLC",  "$I0", 0 },
    { "RLCY",   "RLC",  "$J0", 0 },
    { "RLD",    "RLD",  0, 0 },
    { "RRCR",   "RRC",  "$C0", 0 },
    { "RRCX",   "RRC",  "$I0", 0 },
    { "RRCY",   "RRC",  "$J0", 0 },
    { "RRD",    "RRD",  0, 0 },
    { "SBCD",   "LD",   "$B0", "BC" },
    { "SBCX",   "SBC",  "A", "$I0" },
    { "SBCY",   "SBC",  "A", "$J0" },
    { "SDED",   "LD",   "$B0", "DE" },
    { "SETB",   "SET",  "$A0", "$D1" },
    { "SETX",   "SET",  "$A0", "$I1" },
    { "SETY",   "SET",  "$A0", "$J1" },
    { "SIXD",   "LD",   "$B0", "IX" },
    { "SIYD",   "LD",   "$B0", "IY" },
    { "SLAR",   "SLA",  "$C0", 0 },
    { "SLAX",   "SLA",  "$I0", 0 },
    { "SLAY",   "SLA",  "$J0", 0 },
    { "SPIX",   "LD",   "SP", "IX" },
    { "SPIY",   "LD",   "SP", "IY" },
    { "SRAR",   "SRA",  "$C0", 0 },
    { "SRAX",   "SRA",  "$I0", 0 },
    { "SRAY",   "SRA",  "$J0", 0 },
    { "SRLR",   "SRL",  "$C0", 0 },
    { "SRLX",   "SRL",  "$I0", 0 },
    { "SRLY",   "SRL",  "$J0", 0 },
    { "SSPD",   "LD",   "$B0", "SP" },
    { "STAI",   "LD",   "I", "A" },
    { "STAR",   "LD",   "R", "A" },
    { "STX",    "LD",   "$I1", "$C0" },
    { "STY",    "LD",   "$J1", "$C0" },
    { "SUBX",   "SUB",  "$I0", 0 },
    { "SUBY",   "SUB",  "$J0", 0 },
    { "XORX",   "XOR",  "$I0", 0 },
    { "XORY",   "XOR",  "$J0", 0 },
    { "XTIX",   "EX",   "(SP)", "IX" },
    { "XTIY",   "EX",   "(SP)", "IY" }
};
static const int lib_size = (int) (sizeof(lib_table) / sizeof(entry_t));

static entry_t *
lookup(const char *arg, entry_t *table, int size) {
    // -- binary search
    int l = 0;
    int r = size;
    while (l < r) {
        int m = (r - l) / 2 + l;
        int cmp = strcasecmp(arg, table[m].orig);
        if (cmp < 0) {
            r = m;
        } else if (cmp > 0) {
            l = m + 1;
        } else {
            return &table[m];
        }
    }
    return NULL;
}

entry_t *
lookup_opcode(const char *arg) {
    return lookup(arg, opcode_table, opcode_size);
}

entry_t *
lookup_libcode(const char *arg) {
    return lookup(arg, lib_table, lib_size);
}

//
// -- register sets
// -- set 0 - B, C, D, E, H, L, A
// -- set 1 - B, C, D, E, H, L, M, A ; M -> (HL))
// -- set 2 - B, D                   ; B -> (BC), D -> (DE)
// -- set 3 - B, D, H, SP            ; B -> BC, D -> DE, H -> HL, SP -> SP
// -- set 4 - B, D, H, PSW           ; B -> BC, D -> DE, H -> HL, PSW -> AF
// -- set 5 - BC, DE, HL, SP
// -- set 6 - BC, DE, IX, SP
// -- register set 7 - BC, DE, IY, SP

typedef struct {
    const char orig[8];
} set0_t;

typedef struct {
    const char orig[8];
    const char repl[8];
} set1_t;

static set0_t set0_table[] = {
    { "A" },
    { "B" },
    { "C" },
    { "D" },
    { "E" },
    { "H" },
    { "L" }
};
static const int set0_size = (int) (sizeof(set0_table) / sizeof(set0_t));


static set1_t set2_table[] = {
    { "B", "(BC)" },
    { "D", "(DE)" }
};
static const int set2_size = (int) (sizeof(set2_table) / sizeof(set1_t));

static set1_t set3_table[] = {
    { "B",   "BC" },
    { "D",   "DE" },
    { "H",   "HL" },
    { "SP",  "SP" }
};
static const int set3_size = (int) (sizeof(set3_table) / sizeof(set1_t));

static set1_t set4_table[] = {
    { "B",   "BC" },
    { "D",   "DE" },
    { "H",   "HL" },
    { "PSW", "AF" }
};
static const int set4_size = (int) (sizeof(set4_table) / sizeof(set1_t));

static set0_t set5_table[] = {
    { "BC" },
    { "DE" },
    { "HL" },
    { "SP" }
};
static const int set5_size = (int) (sizeof(set5_table) / sizeof(set0_t));

static set0_t set6_table[] = {
    { "BC" },
    { "DE" },
    { "IX" },
    { "SP" }
};
static const int set6_size = (int) (sizeof(set6_table) / sizeof(set0_t));

static set0_t set7_table[] = {
    { "BC" },
    { "DE" },
    { "IY" },
    { "SP" }
};
static const int set7_size = (int) (sizeof(set7_table) / sizeof(set0_t));

static const char *
lookup_set0(const char *arg, set0_t *table, int size) {
    for (int i = 0; i < size; ++i) {
        if (strcasecmp(arg, table[i].orig) == 0) {
            return table[i].orig;
        }
    }
    warning("invalid register");
    return arg;
}

static const char *
lookup_set1(const char *arg, set1_t *table, int size) {
    for (int i = 0; i < size; ++i) {
        if (strcasecmp(arg, table[i].orig) == 0) {
            return table[i].repl;
        }
    }
    warning("invalid register");
    return arg;
}

const char *
lookup_reg_set0(const char *arg) {
    return lookup_set0(arg, set0_table, set0_size);
}

const char *
lookup_reg_set1(const char *arg) {
    if (strcasecmp(arg, "M") == 0) {
        return "(HL)";
    }
    return lookup_set0(arg, set0_table, set0_size);
}

const char *
lookup_reg_set2(const char *arg) {
    return lookup_set1(arg, set2_table, set2_size);
}

const char *
lookup_reg_set3(const char *arg) {
    return lookup_set1(arg, set3_table, set3_size);
}

const char *
lookup_reg_set4(const char *arg) {
    return lookup_set1(arg, set4_table, set4_size);
}

const char *
lookup_reg_set5(const char *arg) {
    return lookup_set0(arg, set5_table, set5_size);
}

const char *
lookup_reg_set6(const char *arg) {
    return lookup_set0(arg, set6_table, set6_size);
}

const char *
lookup_reg_set7(const char *arg) {
    return lookup_set0(arg, set7_table, set7_size);
}
