/* parse.h - parser for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#include <stdio.h>

#include <bcc.h>
#include <bcc/types.h>
#include <bcc/parser.h>

struct nodestruct *etptr = NULL;     /* ptr to next entry in expression tree */
struct symstruct *gvarsymptr = NULL; /* gsymptr for last identifier declared */
scopelev_t level = 0;             /* scope level depends on zero init */
