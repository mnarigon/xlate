xlate-8080 - translate Intel i8080 assembly language mnemonics to Zilog Z80 mnemonics

usage: xlate-8080 [-cl] [-o file] [file]
  translates i8080 mnemonics to the equivalent Z80 mnemonics
  reads from file (or stdin if file not given on command line)
    and translates the source, writing to stdout (or file specified
    by the -o option)
options:
  -c honor source file uppercase/lowercase usage
  -l translate z80.lib macros
  -o file output translation to file

xlate-z80 - translate Zilog Z80 assembly language mnemonics to Intel i8080 mnemonics

usage: xlate-z80 [-c] [-o file] [file]
  translates Z80 mnemonics to the equivalent i8080 mnemonics
  reads from file (or stdin if file not given on command line)
    and translates the source, writing to stdout (or file specified
    by the -o option)
note:
  xlate-z80 will by default generate z80.lib macros
options:
  -c honor source file uppercase/lowercase usage
  -o file output translation to file
