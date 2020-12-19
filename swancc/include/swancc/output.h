/* output.h - output for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#ifndef _SWANCC_SWANCC_OUTPUT_H
#define _SWANCC_SWANCC_OUTPUT_H

#define OUTBYTE(ch) do { char *outp = outbufptr; \
                         *outp++ = (ch);                  \
                         outbufptr = outp;                \
                         if (outp >= outbuftop) { flushout(); }  } while (0)

extern bool_t ctext;        /* nonzero to intermix C source depends on zero init */
extern char *outbufptr;     /* current spot in output buffer */
extern char *outbuftop;     /* top of current output buffer */
extern bool_t watchlc;      /* nonzero to print lc after every line depends on zero init */

/* Prototypes */
void bugerror(char *message);
void error(char *message);
void fatalerror(char *message);
void limiterror(char *message);

/* TODO: Most of these function need to be removed and replace by standard IO */
void openout(char *oname);

void closeout(void) __attribute__ ((deprecated));
void error2error(char *message1, char *message2);
void finishup(void) __attribute__ ((deprecated));
void flushout(void) __attribute__ ((deprecated));
void initout(void) __attribute__ ((deprecated));
void outbyte(int ch) __attribute__ ((deprecated));
void outcomma(void) __attribute__ ((deprecated));
void outcpplinenumber(uint32_t nr, char *fname, char *str);
void outhex(uoffset_T num) __attribute__ ((deprecated));
void outhexdigs(uoffset_T num) __attribute__ ((deprecated));
void outline(char *s) __attribute__ ((deprecated));
void outminus(void) __attribute__ ((deprecated));
void outnl(void) __attribute__ ((deprecated));
void outnbyte(int byte) __attribute__ ((deprecated));
void outnhex(uoffset_T num) __attribute__ ((deprecated));
void outnop1str(char *s) __attribute__ ((deprecated));
void outnop2str(char *s) __attribute__ ((deprecated));
void outnstr(char *s) __attribute__ ((deprecated));
void outop0str(char *s) __attribute__ ((deprecated));
void outop1str(char *s) __attribute__ ((deprecated));
void outop2str(char *s) __attribute__ ((deprecated));
void outop3str(char *s) __attribute__ ((deprecated));
void outplus(void) __attribute__ ((deprecated));
void outshex(offset_T num) __attribute__ ((deprecated));
void outstr(char *s) __attribute__ ((deprecated));
void outtab(void) __attribute__ ((deprecated));
void outudec(uint32_t num) __attribute__ ((deprecated));
void outuvalue(uvalue_t num) __attribute__ ((deprecated));
void outvalue(value_t num) __attribute__ ((deprecated));

char *pushudec(char *s, uint32_t num);
void setoutbufs(void);

#endif /* _SWANCC_SWANCC_OUTPUT_H */
