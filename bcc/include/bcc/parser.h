/* parse.h - parser for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _BCC_BCC_PARSE_H
#define _BCC_BCC_PARSE_H

/* possible scope levels */
#define ARGLEVEL    1
#define GLBLEVEL    0
#define MAXLEVEL    125
#define MINLOCLEVEL 1

/* possible node flags */
#define LVALUE      (1U << 0)

extern struct nodestruct *etptr;     /* ptr to next entry in expression tree */
extern struct symstruct *gvarsymptr; /* gsymptr for last identifier declared */
extern scopelev_t level;             /* scope level depends on zero init */

#endif /* _BCC_BCC_PARSE_H */
