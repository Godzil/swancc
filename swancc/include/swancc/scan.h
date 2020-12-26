/* scan.h - scanner for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_SCAN_H
#define _SWANCC_SWANCC_SCAN_H

#define NAMESIZE    64    /* limit on identifier lengths */
#define SYMOFCHAR(ch)    (symofchar[(uint8_t) (ch)])

extern op_t arg1op;        /* LISTOP, or ROOTLISTOP if arg1inreg */
extern struct constant_t
{
    union
    {
        char *s;
        value_t v;
        double d;
    } value;
    struct typestruct *type;
} constant;            /* value of last constant scanned */

/* sym tells type */
extern char funcname[NAMESIZE];    /* name of current function for unique labels */
extern char gs2name[2 + NAMESIZE]; /* 2 reserved for namespace keys */
#define gsname (gs2name + 2)       /* before last identifier */
extern struct symstruct *gsymptr;  /* symbol ptr for last identifier */
extern bool_t incppexpr;           /* nonzero while scanning cpp expression */
extern sym_t sym;                  /* current symbol */
extern sym_t symofchar[];          /* table to convert chars to their symbols */
extern bool_t expect_statement;    /* If set #asm needs to clear the recursive pending operations. ie: if stmts. */

/* Prototypes */
void cppscan(int asmonly);
void eofin(char *message);
bool_t isident(void);
void nextsym(void);
void stringorcharconst(void);

#endif /* _SWANCC_SWANCC_SCAN_H */
