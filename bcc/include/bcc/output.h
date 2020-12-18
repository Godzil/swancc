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

/* Prototypes */
void bugerror(char *message);
void closeout(void);
void error(char *message);
void error2error(char *message1, char *message2);
void fatalerror(char *message);
void finishup(void);
void flushout(void);
void limiterror(char *message);
void initout(void);
void openout(char *oname);
void outbyte(int ch);
void outcomma(void);
void outcpplinenumber(unsigned nr, char *fname, char *str);
void outhex(uoffset_T num);
void outhexdigs(uoffset_T num);
void outline(char *s);
void outminus(void);
void outnl(void);
void outnbyte(int byte);
void outnhex(uoffset_T num);
void outnop1str(char *s);
void outnop2str(char *s);
void outnstr(char *s);
void outop0str(char *s);
void outop1str(char *s);
void outop2str(char *s);
void outop3str(char *s);
void outplus(void);
void outshex(offset_T num);
void outstr(char *s);
void outtab(void);
void outudec(unsigned num);
void outuvalue(uvalue_t num);
void outvalue(value_t num);
char *pushudec(char *s, unsigned num);
void setoutbufs(void);

#endif /* _BCC_BCC_OUTPUT_H */
