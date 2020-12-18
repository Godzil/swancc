/* glogcode.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _BCC_BCC_GLOGCODE_H
#define _BCC_BCC_GLOGCODE_H

void cmp(struct symstruct *source, struct symstruct *target, ccode_t *pcondtrue);
void condop(struct nodestruct *exp);
void jumpfalse(struct nodestruct *exp, label_no label);
void jumptrue(struct nodestruct *exp, label_no label);
void logop(struct nodestruct *exp);

#endif /* _BCC_BCC_GLOGCODE_H */
