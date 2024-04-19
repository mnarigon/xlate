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
#include "warning.h"

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
// -- generate an argument according to the argument code
//
// -- argument code
// -- X - literal
// -- $An - nth argument
// -- $Hn - argument is 0-7, multiply by 8 and output as hex
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
        case 'H': {
            // -- handle the RST offset
            int value = scan_number(select_arg(arg[2])) / 8;
            fprintf(out_fp, "%d", value);
            break;
        }
    }
}

static void
prep_arguments(buffer_t *line, entry_t *entry) {
    // -- output the input line up to the mark
    buffer_putline(line, get_pmark(), get_mark(line), out_fp);
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
    
    // -- backup over whitespace
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
    // -- and process the argument
    if (entry->arg0[0]) {
        putstr("\t");
        do_argument(entry->arg0);
    }
}

static const char *adc_sbc[][5] = {
    { "ADC\t", "ADCX\t", "ADCY\t", "ACI\t", "DADC\t" },
    { "SBB\t", "SBCX\t", "SBCY\t", "SBI\t", "DSBC\t" },
};

//
// -- handle ADC, SBC
void
do_adc_sbc(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    // -- scan the arguments for their type codes
    const char *arg0_str = get_str(&argument0);
    const char *arg1_str = get_str(&argument1);
    int arg0_code = lookup_arg_code(arg0_str);
    int arg1_code = lookup_arg_code(arg1_str);
    int index = entry->arg0[0] - '0';

    // -- select opcode from arguments
    if (is_null(arg1_code)) {
        // -- single argument, assume implicit reg A
        if (is_set1(arg0_code)) {
            putcasestr(adc_sbc[index][0]);
            putcasestr(lookup_set1(arg0_str));
        } else if (is_IX_indexed(arg0_code)) {
            putcasestr(adc_sbc[index][1]);
            putstr(scan_displacement(&argument0));
        } else if (is_IX_indexed(arg0_code)) {
            putcasestr(adc_sbc[index][2]);
            putstr(scan_displacement(&argument0));
        } else {
            putcasestr(adc_sbc[index][3]);
            putstr(arg0_str);
        }
    } else if (isA(arg0_code)) {
        if (is_set1(arg1_code)) {
            putcasestr(adc_sbc[index][0]);
            putcasestr(lookup_set1(arg1_str));
        } else if (is_IX_indexed(arg1_code)) {
            putcasestr(adc_sbc[index][1]);
            putstr(scan_displacement(&argument1));
        } else if (is_IY_indexed(arg1_code)) {
            putcasestr(adc_sbc[index][2]);
            putstr(scan_displacement(&argument1));
        } else {
            putcasestr(adc_sbc[index][3]);
            putstr(arg1_str);
        }
    } else if (isHL(arg0_code) && is_set5(arg1_code)) {
        putcasestr(adc_sbc[index][4]);
        putcasestr(arg1_str);
    } else {
        warning("can't select opcode from arguments");
        last_pmark();
    }
}

//
// -- handle ADD
void
do_add(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    // -- scan the arguments for their type codes
    const char *arg0_str = get_str(&argument0);
    const char *arg1_str = get_str(&argument1);
    int arg0_code = lookup_arg_code(arg0_str);
    int arg1_code = lookup_arg_code(arg1_str);

    // -- select opcode from arguments
    if (is_null(arg1_code)) {
        // -- single argument, assume implicit reg A
        if (is_set1(arg0_code)) {
            putcasestr("ADD\t");
            putcasestr(lookup_set1(arg0_str));
        } else if (is_IX_indexed(arg0_code)) {
            putcasestr("ADDX\t");
            putstr(scan_displacement(&argument0));
        } else if (is_IY_indexed(arg0_code)) {
            putcasestr("ADDY\t");
            putstr(scan_displacement(&argument0));
        } else {
            putcasestr("ADI\t");
            putstr(arg0_str);
        }
    } else if (isA(arg0_code)) {
        if (is_set1(arg1_code)) {
            putcasestr("ADD\t");
            putcasestr(lookup_set1(arg1_str));
        } else if (is_IX_indexed(arg1_code)) {
            putcasestr("ADDX\t");
            putstr(scan_displacement(&argument1));
        } else if (is_IY_indexed(arg1_code)) {
            putcasestr("ADDY\t");
            putstr(scan_displacement(&argument1));
        } else {
            putcasestr("ADI\t");
            putstr(arg1_str);
        }
    } else if (isHL(arg0_code) && is_set3(arg1_code)) {
        putcasestr("DAD\t");
        putcasestr(lookup_set3(arg1_str));
    } else if (isIX(arg0_code) && is_set6(arg1_code)) {
        putcasestr("DADX\t");
        putcasestr(arg1_str);
    } else if (isIY(arg0_code) && is_set7(arg1_code)) {
        putcasestr("DADY\t");
        putcasestr(arg1_str);
    } else {
        warning("can't select opcode from arguments");
        last_pmark();
    }
}

static const char *dec_inc[][6] = {
    { "DCR\t", "DCX\t", "DCRX\t", "DCRY\t", "DCRXIX\t", "DCRXIY\t" },
    { "INR\t", "INX\t", "INRX\t", "INRY\t", "INRXIX\t", "INRXIY\t" },
};

//
// -- handle DEC/INC
void
do_dec_inc(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    // -- scan the arguments for their type codes
    const char *arg0_str = get_str(&argument0);
    int arg0_code = lookup_arg_code(arg0_str);
    int index = entry->arg0[0] - '0';

    if (is_set1(arg0_code)) {
        putcasestr(dec_inc[index][0]);
        putcasestr(lookup_set1(arg0_str));
    } else if (is_set3(arg0_code)) {
        putcasestr(dec_inc[index][1]);
        putcasestr(lookup_set3(arg0_str));
    } else if (is_IX_indexed(arg0_code)) {
        putcasestr(dec_inc[index][2]);
        putstr(scan_displacement(&argument0));
    } else if (is_IY_indexed(arg0_code)) {
        putcasestr(dec_inc[index][3]);
        putstr(scan_displacement(&argument0));
    } else if (isIX(arg0_code)) {
        putcasestr(dec_inc[index][4]);
    } else if (isIY(arg0_code)) {
        putcasestr(dec_inc[index][5]);
    } else {
        warning("can't select opcode from arguments");
        last_pmark();
    }
}

static const char *operi[][4] = {
    { "ANA\t", "ANDX\t", "ANDY\t", "ANI\t" },
    { "CMP\t", "CMPX\t", "CMPY\t", "CPI\t" },
    { "ORA\t", "ORX\t",  "ORY\t",  "ORI\t" },
    { "SUB\t", "SUBX\t", "SUBY\t", "SUI\t" },
    { "XRA\t", "XORX\t", "XORY\t", "XRI\t" },
};

//
// -- AND, CP, OR, SUB, XOR
void
do_operi(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    // -- scan the arguments for their type codes
    const char *arg0_str = get_str(&argument0);
    const char *arg1_str = get_str(&argument1);
    int arg0_code = lookup_arg_code(arg0_str);
    int arg1_code = lookup_arg_code(arg1_str);
    int index = entry->arg0[0] - '0';
    
    if (isA(arg0_code) && !is_null(arg1_code)) {
        // -- ignore explicit reg A
        if (is_set1(arg1_code)) {
            putcasestr(operi[index][0]);
            putcasestr(lookup_set1(arg1_str));
        } else if (is_IX_indexed(arg1_code)) {
            putcasestr(operi[index][1]);
            putstr(scan_displacement(&argument1));
        } else if (is_IY_indexed(arg1_code)) {
            putcasestr(operi[index][2]);
            putstr(scan_displacement(&argument1));
        } else {
            putcasestr(operi[index][3]);
            putstr(arg1_str);
        }
    } else if (is_set1(arg0_code)) {
        putcasestr(operi[index][0]);
        putcasestr(lookup_set1(arg0_str));
    } else if (is_IX_indexed(arg0_code)) {
        putcasestr(operi[index][1]);
        putstr(scan_displacement(&argument0));
    } else if (is_IY_indexed(arg0_code)) {
        putcasestr(operi[index][2]);
        putstr(scan_displacement(&argument0));
    } else {
        putcasestr(operi[index][3]);
        putstr(arg0_str);
    }
}

static const char *opers[][3] = {
    { "RALR\t", "RALX\t", "RALY\t" },
    { "RLCR\t", "RLCX\t", "RLCY\t" },
    { "RARR\t", "RARX\t", "RARY\t" },
    { "RRCR\t", "RRCX\t", "RRCY\t" },
    { "SLAR\t", "SLAX\t", "SLAY\t" },
    { "SRAR\t", "SRAX\t", "SRAY\t" },
    { "SRLR\t", "SRLX\t", "SRLY\t" },
};

// -- RL, RLC, RR, RRC, SLA, SRA, SRL
void
do_opers(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    // -- scan the arguments for their type codes
    const char *arg0_str = get_str(&argument0);
    int arg0_code = lookup_arg_code(arg0_str);
    int index = entry->arg0[0] - '0';

    if (is_set1(arg0_code)) {
        putcasestr(opers[index][0]);
        putcasestr(lookup_set1(arg0_str));
    } else if (is_IX_indexed(arg0_code)) {
        putcasestr(opers[index][1]);
        putstr(scan_displacement(&argument0));
    } else if (is_IY_indexed(arg0_code)) {
        putcasestr(opers[index][2]);
        putstr(scan_displacement(&argument0));
    } else {
        warning("can't select opcode from arguments");
        last_pmark();
    }
}

static const char *bits[][3] = {
    { "BIT\t",  "BITX\t", "BITY\t" },
    { "RES\t",  "RESX\t", "RESY\t" },
    { "SETB\t", "SETX\t", "SETY\t" },
};

// -- BIT, RES, SET
void
do_bits(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    // -- scan the arguments for their type codes
    const char *arg0_str = get_str(&argument0);
    const char *arg1_str = get_str(&argument1);
    int arg1_code = lookup_arg_code(arg1_str);
    int index = entry->arg0[0] - '0';

    if (is_set1(arg1_code)) {
        putcasestr(bits[index][0]);
        putstr("\t");
        putstr(arg0_str);
        putstr(",");
        putcasestr(lookup_set1(arg1_str));
    } else if (is_IX_indexed(arg1_code)) {
        putcasestr(bits[index][1]);
        putstr("\t");
        putstr(arg0_str);
        putstr(",");
        putstr(scan_displacement(&argument1));
    } else if (is_IY_indexed(arg1_code)) {
        putcasestr(bits[index][2]);
        putstr("\t");
        putstr(arg0_str);
        putstr(",");
        putstr(scan_displacement(&argument1));
    } else {
        warning("can't select opcode from arguments");
        last_pmark();
    }
}

// -- CALL
void
do_call(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    const char *arg0_str = get_str(&argument0);
    const char *arg1_str = get_str(&argument1);
    
    if (is_cc(arg0_str)) {
        putcasestr("C");
        putcasestr(arg0_str);
        putstr("\t");
        putstr(arg1_str);
    } else {
        putcasestr("CALL\t");
        putstr(arg0_str);
    }
}

// -- JP
void
do_jp(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    const char *arg0_str = get_str(&argument0);
    const char *arg1_str = get_str(&argument1);
    int arg0_code = lookup_arg_code(arg0_str);
    
    if (is_cc(arg0_str)) {
        putcasestr("J");
        putcasestr(arg0_str);
        putstr("\t");
        putstr(arg1_str);
    } else if (isHLi(arg0_code)) {
        putcasestr("PCHL");
    } else if (isIXi(arg0_code)) {
        putcasestr("PCIX");
    } else if (isIYi(arg0_code)) {
        putcasestr("PCIY");
    } else {
        putcasestr("JMP\t");
        putstr(arg0_str);
    }
}

// -- JR
void
do_jr(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    const char *arg0_str = get_str(&argument0);
    const char *arg1_str = get_str(&argument1);
    
    if (is_jr_cc(arg0_str)) {
        putcasestr("JR");
        putcasestr(arg0_str);
        putstr("\t");
        putstr(arg1_str);
    } else {
        putcasestr("JR\t");
        putstr(arg0_str);
    }
}

// -- RET
void
do_ret(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    const char *arg0_str = get_str(&argument0);
    
    if (is_cc(arg0_str)) {
        putcasestr("R");
        putcasestr(arg0_str);
    } else {
        putcasestr("RET");
    }
}

// -- POP, PUSH
void
do_pop_psh(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    const char *arg0_str = get_str(&argument0);
    int arg0_code = lookup_arg_code(arg0_str);

    if (is_set4(arg0_code)) {
        putcasestr(entry->repl);
        putstr("\t");
        putcasestr(lookup_set4(arg0_str));
    } else if (isIX(arg0_code)) {
        putcasestr(entry->repl);
        putcasestr("IX");
    } else if (isIY(arg0_code)) {
        putcasestr(entry->repl);
        putcasestr("IY");
    } else {
        warning("can't select opcode from arguments");
        last_pmark();
    }
}

// -- EX
void
do_ex(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    const char *arg0_str = get_str(&argument0);
    const char *arg1_str = get_str(&argument1);
    int arg0_code = lookup_arg_code(arg0_str);
    int arg1_code = lookup_arg_code(arg1_str);

    if (isAF(arg0_code) && isAFp(arg1_code)) {
        putcasestr("EXAF");
    } else if (isDE(arg0_code) && isHL(arg1_code)) {
        putcasestr("XCHG");
    } else if (isSPi(arg0_code)) {
        if (isHL(arg1_code)) {
            putcasestr("XTHL");
        } else if (isIX(arg1_code)) {
            putcasestr("XTIX");
        } else if (isIY(arg1_code)) {
            putcasestr("XTIY");
        } else {
            warning("can't select opcode from arguments");
            last_pmark();
        }
    } else {
        warning("can't select opcode from arguments");
        last_pmark();
    }
}

// -- IN
void
do_in(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    const char *arg0_str = get_str(&argument0);
    const char *arg1_str = get_str(&argument1);
    int arg0_code = lookup_arg_code(arg0_str);
    int arg1_code = lookup_arg_code(arg1_str);

    if (is_set0(arg0_code) && isCi(arg1_code)) {
        putcasestr("INP\t");
        putcasestr(arg0_str);
    } else if (isA(arg0_code)) {
        putcasestr("IN\t");
        putstr(scan_indirect(&argument1));
    } else {
        warning("can't select opcode from arguments");
        last_pmark();
    }
}

// -- OUT
void
do_out(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    const char *arg0_str = get_str(&argument0);
    const char *arg1_str = get_str(&argument1);
    int arg0_code = lookup_arg_code(arg0_str);
    int arg1_code = lookup_arg_code(arg1_str);

    if (is_set0(arg1_code) && isCi(arg0_code)) {
        putcasestr("OUTP\t");
        putcasestr(arg1_str);
    } else if (isA(arg1_code)) {
        putcasestr("OUT\t");
        putstr(scan_indirect(&argument0));
    } else {
        warning("can't select opcode from arguments");
        last_pmark();
    }
}

// -- IM
void
do_im(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    const char *arg0_str = get_str(&argument0);

    putcasestr("IM");
    putcasestr(arg0_str);
}

// -- LD
void
do_ld(struct buffer *line, struct lookup *entry) {
    prep_arguments(line, entry);

    const char *arg0_str = get_str(&argument0);
    const char *arg1_str = get_str(&argument1);
    int arg0_code = lookup_arg_code(arg0_str);
    int arg1_code = lookup_arg_code(arg1_str);

    if (isA(arg0_code) && isI(arg1_code)) {
        putcasestr("LDAI");
    } else if (isI(arg0_code) && isA(arg1_code)) {
        putcasestr("STAI");
    } else if (isA(arg0_code) && isR(arg1_code)) {
        putcasestr("LDAR");
    } else if (isR(arg0_code) && isA(arg1_code)) {
        putcasestr("STAR");
    } else if (isSP(arg0_code) && isHL(arg1_code)) {
        putcasestr("SPHL");
    } else if (isSP(arg0_code) && isIX(arg1_code)) {
        putcasestr("SPIX");
    } else if (isSP(arg0_code) && isIY(arg1_code)) {
        putcasestr("SPIY");
    } else if (is_set2(arg0_code) && isA(arg1_code)) {
        putcasestr("STAX\t");
        putcasestr(lookup_set2(arg0_str));
    } else if (isA(arg0_code) && is_set2(arg1_code)) {
        putcasestr("LDAX\t");
        putcasestr(lookup_set2(arg1_str));
    } else if (is_set3(arg0_code) || isIX(arg0_code) || isIY(arg0_code)) {
        if (is_indirect(arg1_code)) {
            if (isBC(arg0_code)) {
                putcasestr("LBCD\t");
            } else if (isDE(arg0_code)) {
                putcasestr("LDED\t");
            } else if (isHL(arg0_code)) {
                putcasestr("LHLD\t");
            } else if (isSP(arg0_code)) {
                putcasestr("LSPD\t");
            } else if (isIX(arg0_code)) {
                putcasestr("LIXD\t");
            } else if (isIY(arg0_code)) {
                putcasestr("LIYD\t");
            }
            putstr(scan_indirect(&argument1));
        } else {
            if (isIX(arg0_code)) {
                putcasestr("LXIX\t");
            } else if (isIY(arg0_code)) {
                putcasestr("LXIY\t");
            } else {
                putcasestr("LXI\t");
                putcasestr(lookup_set3(arg0_str));
                putstr(",");
            }
            putstr(arg1_str);
        }
    } else if (is_set3(arg1_code) || isIX(arg1_code) || isIY(arg1_code)) {
        if (isBC(arg1_code)) {
            putcasestr("SBCD\t");
        } else if (isDE(arg1_code)) {
            putcasestr("SDED\t");
        } else if (isHL(arg1_code)) {
            putcasestr("SHLD\t");
        } else if (isSP(arg1_code)) {
            putcasestr("SSPD\t");
        } else if (isIX(arg1_code)) {
            putcasestr("SIXD\t");
        } else if (isIY(arg1_code)) {
            putcasestr("SIYD\t");
        }
        putstr(scan_indirect(&argument0));
    } else if (is_IX_indexed(arg0_code)) {
        if (is_set0(arg1_code)) {
            putcasestr("STX\t");
            putcasestr(arg1_str);
            putstr(",");
            putstr(scan_displacement(&argument0));
        } else {
            putcasestr("MVIX\t");
            putstr(arg1_str);
            putstr(",");
            putstr(scan_displacement(&argument0));
        }
    } else if (is_IY_indexed(arg0_code)) {
        if (is_set0(arg1_code)) {
            putcasestr("STY\t");
            putcasestr(arg1_str);
            putstr(",");
            putstr(scan_displacement(&argument0));
        } else {
            putcasestr("MVIY\t");
            putstr(arg1_str);
            putstr(",");
            putstr(scan_displacement(&argument0));
        }
    } else if (is_IX_indexed(arg1_code)) {
        putcasestr("LDX\t");
        putcasestr(arg0_str);
        putstr(",");
        putstr(scan_displacement(&argument1));
    } else if (is_IY_indexed(arg1_code)) {
        putcasestr("LDY\t");
        putcasestr(arg0_str);
        putstr(",");
        putstr(scan_displacement(&argument1));
    } else if (is_set1(arg0_code) && is_set1(arg1_code)) {
        putcasestr("MOV\t");
        putcasestr(lookup_set1(arg0_str));
        putstr(",");
        putcasestr(lookup_set1(arg1_str));
    } else if (isA(arg0_code) && is_indirect(arg1_code)) {
        putcasestr("LDA\t");
        putstr(scan_indirect(&argument1));
    } else if (is_indirect(arg0_code) && isA(arg1_code)) {
        putcasestr("STA\t");
        putstr(scan_indirect(&argument0));
    } else if (is_set1(arg0_code)) {
        putcasestr("MVI\t");
        putcasestr(lookup_set1(arg0_str));
        putstr(",");
        putstr(arg1_str);
    } else {
        warning("can't select opcode from arguments");
        last_pmark();
    }
}
