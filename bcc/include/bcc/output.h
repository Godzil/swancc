/* output.h - output for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#ifndef _BCC_BCC_OUTPUT_H
#define _BCC_BCC_OUTPUT_H

#define OUTBYTE(ch) do { char *outp = outbufptr; \
                         *outp++ = (ch);                  \
                         outbufptr = outp;                \
                         if (outp >= outbuftop) { flushout(); }  } while (0)

extern bool_t ctext;        /* nonzero to intermix C source depends on zero init */
extern char *outbufptr;     /* current spot in output buffer */
extern char *outbuftop;     /* top of current output buffer */
extern bool_t watchlc;      /* nonzero to print lc after every line depends on zero init */

#endif /* _BCC_BCC_OUTPUT_H */
