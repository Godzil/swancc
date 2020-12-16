/* output.h - output for bcc */

/* Copyright (C) 1992 Bruce Evans */

#define OUTBYTE(ch) \
    do {register char *outp = outbufptr; *outp++ = (ch); \
	outbufptr = outp; if (outp >= outbuftop) flushout(); } while (0)

extern bool_t ctext;		/* nonzero to intermix C source */
				/* depends on zero init */
extern char *outbufptr;		/* current spot in output buffer */
extern char *outbuftop;		/* top of current output buffer */
extern bool_t watchlc;		/* nonzero to print lc after every line */
				/* depends on zero init */
