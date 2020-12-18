/* sc.h - storage classes for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _BCC_BCC_SC_H
#define _BCC_BCC_SC_H

#ifdef MC6809
#define DIRECTPAGE  0x01    /* modifier on global to show in dpseg */
#endif

#define externAL    0x02    /* external */
#define STATIC      0x04

/* symbols with flags above the 1st initialised value are not to be dumped */
#define MAXDUMPFLAG 0x07
#define INITIALIZED 0x08    /* modifier on global to show initialized */
#define LABELLED    0x10    /* modifier on static to show labelled and on STRING to show labelled */

/* Remaining "flags" are numbers, not flags
 * they are disjoint from all combinations of flags above
 * except STRING => LABELLED (and no other flags)
 */

#define DEFINITION 0x20        /* #defined name */
#define KEYWORD    0x40        /* reserved word for C */
#define STRUCTNAME 0x60        /* struct/union name or member name */
#define REGVAR     0x80        /* register variable */
#define TEMP       0xa0        /* temporary on stack expression eval */
#define STRING     0xc0        /* string constant (=> LABELLED) */

/* Protoypes */
int main(int argc, char *argv[]);

#endif /* _BCC_BCC_SC_H */
