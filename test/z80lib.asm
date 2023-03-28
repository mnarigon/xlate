	title	'z80.lib macros'
	maclib	z80	; comment after translation
	org	0100h

	adcx	dd
	adcy	dd
	addx	dd
	addy	dd
	andx	dd
	andy	dd
	bit	0,a
	bitx	0,dd
	bity	0,dd
	ccd
	ccdr
	cci
	ccir
	cmpx	dd
	cmpy	dd
	dadc	bc
	dadx	bc
	dady	bc
	dcrx	dd
	dcry	dd
	dcxix
	dcxiy
label0:
	djnz	label0
	dsbc	bc
	exaf
	exx
	im0
	im1
	im2
	ind
	indr
	ini
	inir
	inp	b
	inrx	dd
	inry	dd
	inxix
	inxiy
label1:
	jr	label1
	jrc	label1
	jrnc	label1
	jrnz	label1
	jrz	label1
	lbcd	nnnn
	ldai
	ldar
	ldd
	lddr
	lded	nnnn
	ldi
	ldir
	ldx	a,dd
	ldy	a,dd
	lixd	nnnn
	liyd	nnnn
	lspd	nnnn
	lxix	nnnn
	lxiy	nnnn
	mvix	nn,dd
	mviy	nn,dd
	neg
	orx	dd
	ory	dd
	outd
	outdr
	outi
	outir
	outp	b
	pcix
	pciy
	popix
	popiy
	pushix
	pushiy
	ralr	a
	ralx	dd
	raly	dd
	rarr	a
	rarx	dd
	rary	dd
	res	0,a
	resx	0,dd
	resy	0,dd
	reti
	retn
	rlcr	a
	rlcx	dd
	rlcy	dd
	rld
	rrcr	a
	rrcx	dd
	rrcy	dd
	rrd
	sbcd	nnnn
	sbcx	dd
	sbcy	dd
	sded	nnnn
	setb	0,a
	setx	0,dd
	sety	0,dd
	sixd	nnnn
	siyd	nnnn
	slar	a
	slax	dd
	slay	dd
	spix
	spiy
	srar	a
	srax	dd
	sray	dd
	srlr	a
	srlx	dd
	srly	dd
	sspd	nnnn
	stai
	star
	stx	a,dd
	sty	a,dd
	subx	dd
	suby	dd
	xorx	dd
	xory	dd
	xtix
	xtiy

;test registers
	ldx	a,dd
	ldx	b,dd
	ldx	c,dd
	ldx	d,dd
	ldx	e,dd
	ldx	h,dd
	ldx	l,dd

	dadc	bc
	dadc	de
	dadc	hl
	dadc	sp

	dsbc	bc
	dsbc	de
	dsbc	hl
	dsbc	sp

	dadx	bc
	dadx	de
	dadx	ix
	dadx	sp

	dady	bc
	dady	de
	dady	iy
	dady	sp

;equates
nnnn	equ	1234h
nn	equ	56h
dd	equ	78h
	end
