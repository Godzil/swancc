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

#endif /* _BCC_BCC_LABEL_H */
