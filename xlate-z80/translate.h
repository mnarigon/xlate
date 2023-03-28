//
// -- translate.h
//
#ifndef TRANSLATE_H
#define TRANSLATE_H

// -- forward declaration
struct buffer;
struct lookup;

//
// -- substitute opcodes according to the lookup table entry
void do_subst(struct buffer *line, struct lookup *entry);

// -- handle ADC, SBC
void do_adc_sbc(struct buffer *line, struct lookup *entry);

// -- handle ADD
void do_add(struct buffer *line, struct lookup *entry);

// -- handle DEC/INC
void do_dec_inc(struct buffer *line, struct lookup *entry);

// -- AND, CP, OR, SUB, XOR
void do_operi(struct buffer *line, struct lookup *entry);

// -- RL, RLC, RR, RRC, SLA, SRA, SRL
void do_opers(struct buffer *line, struct lookup *entry);

// -- BIT, RES, SET
void do_bits(struct buffer *line, struct lookup *entry);

// -- CALL
void do_call(struct buffer *line, struct lookup *entry);

// -- JP
void do_jp(struct buffer *line, struct lookup *entry);

// -- JR
void do_jr(struct buffer *line, struct lookup *entry);

// -- RET
void do_ret(struct buffer *line, struct lookup *entry);

// -- POP, PUSH
void do_pop_psh(struct buffer *line, struct lookup *entry);

// -- EX
void do_ex(struct buffer *line, struct lookup *entry);

// -- IN
void do_in(struct buffer *line, struct lookup *entry);

// -- OUT
void do_out(struct buffer *line, struct lookup *entry);

// -- IM
void do_im(struct buffer *line, struct lookup *entry);

// -- LD
void do_ld(struct buffer *line, struct lookup *entry);

#endif
