/* scan.h - global variables for scanner for assembler */

#define EOLCHAR '\n'

extern struct sym_s *gsymptr;	/* global symbol ptr */
extern char lindirect;		/* left symbol for indirect addressing */
extern char *lineptr;		/* current line position */
extern offset_t number; 	/* constant number */
extern char * rindexp;		/* error code for missing rindirect */
extern char rindirect;		/* right symbol for indirect addressing */
extern char sym;		/* current symbol */
extern char *symname;		/* current symbol name */
