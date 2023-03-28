	title 'i8080 mnemonics'
	org	0100H
;00
	nop
	lxi	b,nnnn
	stax	b
	inx	b
	inr	b
	dcr	b
	mvi	b,nn
	rlc
	db	008h	;undefined
	dad	b
	ldax	b
	dcx	b
	inr	c
	dcr	c
	mvi	c,nn
	rrc
;10
	db	010h	;undefined
	lxi	d,nnnn
	stax	d
	inx	d
	inr	d
	dcr	d
	mvi	d,nn
	ral
	db	018h	;undefined
	dad	d
	ldax	d
	dcx	d
	inr	e
	dcr	e
	mvi	e,nn
	rar
;20
	db	020h	;undefined
	lxi	h,nnnn
	shld	nnnn
	inx	h
	inr	h
	dcr	h
	mvi	h,nn
	daa
	db	028h	;undefined
	dad	h
	lhld	nnnn
	dcx	h
	inr	l
	dcr	l
	mvi	l,nn
	cma
;30
	db	030h	;undefined
	lxi	sp,nnnn
	sta	nnnn
	inx	sp
	inr	m
	dcr	m
	mvi	m,nn
	stc
	db	038h	;undefined
	dad	sp
	lda	nnnn
	dcx	sp
	inr	a
	dcr	a
	mvi	a,nn
	cmc
;40
	mov	b,b
	mov	b,c
	mov	b,d
	mov	b,e
	mov	b,h
	mov	b,l
	mov	b,m
	mov	b,a
	mov	c,b
	mov	c,c
	mov	c,d
	mov	c,e
	mov	c,h
	mov	c,l
	mov	c,m
	mov	c,a
;50
	mov	d,b
	mov	d,c
	mov	d,d
	mov	d,e
	mov	d,h
	mov	d,l
	mov	d,m
	mov	d,a
	mov	e,b
	mov	e,c
	mov	e,d
	mov	e,e
	mov	e,h
	mov	e,l
	mov	e,m
	mov	e,a
;60
	mov	h,b
	mov	h,c
	mov	h,d
	mov	h,e
	mov	h,h
	mov	h,l
	mov	h,m
	mov	h,a
	mov	l,b
	mov	l,c
	mov	l,d
	mov	l,e
	mov	l,h
	mov	l,l
	mov	l,m
	mov	l,a
;70
	mov	m,b
	mov	m,c
	mov	m,d
	mov	m,e
	mov	m,h
	mov	m,l
	hlt
	mov	m,a
	mov	a,b
	mov	a,c
	mov	a,d
	mov	a,e
	mov	a,h
	mov	a,l
	mov	a,m
	mov	a,a
;80
	add	b
	add	c
	add	d
	add	e
	add	h
	add	l
	add	m
	add	a
	adc	b
	adc	c
	adc	d
	adc	e
	adc	h
	adc	l
	adc	m
	adc	a
;90
	sub	b
	sub	c
	sub	d
	sub	e
	sub	h
	sub	l
	sub	m
	sub	a
	sbb	b
	sbb	c
	sbb	d
	sbb	e
	sbb	h
	sbb	l
	sbb	m
	sbb	a
;a0
	ana	b
	ana	c
	ana	d
	ana	e
	ana	h
	ana	l
	ana	m
	ana	a
	xra	b
	xra	c
	xra	d
	xra	e
	xra	h
	xra	l
	xra	m
	xra	a
;b0
	ora	b
	ora	c
	ora	d
	ora	e
	ora	h
	ora	l
	ora	m
	ora	a
	cmp	b
	cmp	c
	cmp	d
	cmp	e
	cmp	h
	cmp	l
	cmp	m
	cmp	a
;c0
	rnz
	pop	b
	jnz	nnnn
	jmp	nnnn
	cnz	nnnn
	push	b
	adi	nn
	rst	0
	rz
	ret
	jz	nnnn
	db	0cbh	;undefined
	cz	nnnn
	call	nnnn
	aci	nn
	rst	1
;d0
	rnc
	pop	d
	jnc	nnnn
	out	nn
	cnc	nnnn
	push	d
	sui	nn
	rst	2
	rc
	db	0d9h	;undefined
	jc	nnnn
	in	nn
	cc	nnnn
	db	0ddh	;undefined
	sbi	nn
	rst	3
;e0
	rpo
	pop	h
	jpo	nnnn
	xthl
	cpo	nnnn
	push	h
	ani	nn
	rst	4
	rpe
	pchl
	jpe	nnnn
	xchg
	cpe	nnnn
	db	0edh	;undefined
	xri	nn
	rst	5
;f0
	rp
	pop	psw
	jp	nnnn
	di
	cp	nnnn
	push	psw
	ori	nn
	rst	6
	rm
	sphl
	jm	nnnn
	ei
	cm	nnnn
	db	0fdh	;undefined
	cpi	nn
	rst	7
;equates
nnnn	equ    01234h
nn	equ    056h
	end
