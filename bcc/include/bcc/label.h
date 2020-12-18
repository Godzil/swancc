/* label.h - assembler-specific label characters for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#ifndef _BCC_BCC_LABEL_H
#define _BCC_BCC_LABEL_H

/* defaults */
#define CCNAMEPREFIX   '_'
#define LABELENDCHAR   ':'    /* last char of ALL labels */
#define LABELSTARTCHAR '.'    /* first char of names of generated labels */
#define LOCALSTARTCHAR '.'    /* first char of local names */
#define PUBLICENDCHAR  ':'

/* adjustments for other assemblers */
#ifdef AS09
#undef LABELENDCHAR
#endif

#ifdef XENIX_AS
#undef LABELSTARTCHAR
#define LABELSTARTCHAR 'L'
#undef LOCALSTARTCHAR
#define LOCALSTARTCHAR 'L'
#endif

/* Protypes */
void bumplc(void);
void bumplc2(void);
void bumplc3(void);
void clearfunclabels(void);
void clearlabels(char *patchbuf, char *patchtop);
void clearswitchlabels(void);
uoffset_T getlc(void);
void deflabel(label_no label);
label_no gethighlabel(void);
label_no getlabel(void);
void jump(label_no label);
void lbranch(ccode_t cond, label_no label);
struct symstruct *namedlabel(void);
void outcond(ccode_t cond);
void outlabel(label_no label);
void outnlabel(label_no label);
void sbranch(ccode_t cond, label_no label);
void unbumplc(void);

#endif /* _BCC_BCC_LABEL_H */
