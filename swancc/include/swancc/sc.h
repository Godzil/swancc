/* sc.h - storage classes for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_SC_H
#define _SWANCC_SWANCC_SC_H

typedef enum sc_t
{
    EXTERNAL    = 0x02,    /* external */
    STATIC      = 0x04,

    /* symbols with flags above the 1st initialised value are not to be dumped */
    MAXDUMPFLAG = 0x07,
    INITIALIZED = 0x08,    /* modifier on global to show initialized */
    LABELLED    = 0x10,    /* modifier on static to show labelled and on STRING to show labelled */

    /* Remaining "flags" are numbers, not flags
     * they are disjoint from all combinations of flags above
     * except STRING => LABELLED (and no other flags)
     */
    DEFINITION = 0x20,        /* #defined name */
    KEYWORD    = 0x40,        /* reserved word for C */
    STRUCTNAME = 0x60,        /* struct/union name or member name */
    REGVAR     = 0x80,        /* register variable */
    TEMP       = 0xa0,        /* temporary on stack expression eval */
    STRING     = 0xc0,        /* string constant (=> LABELLED) */
} sc_t;

/* Protoypes */
int main(int argc, char *argv[]);

#endif /* _SWANCC_SWANCC_SC_H */
