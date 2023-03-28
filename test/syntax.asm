; i8080 syntax tests
; line numbers
0001
0002label
0003label adc b
0004 adc b
0005adc:
0006adc: adc b

; star comment
0001* adc b this is a star comment
* adc c this is a star comment

; multiple statements
label xra a ! add b ! adc c

; string arguments
db 'this is a string don''t bother',0
