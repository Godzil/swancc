/* hardop.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _BCC_BCC_HARDOP_H
#define _BCC_BCC_HARDOP_H

void add(struct symstruct *source, struct symstruct *target);
void incdec(op_pt op, struct symstruct *source);
void neg(struct symstruct *target);
void not (struct symstruct *target);
void op1(op_pt op, struct symstruct *source, struct symstruct *target);
void ptrsub(struct symstruct *source, struct symstruct *target);
void sub(struct symstruct *source, struct symstruct *target);

#endif /* _BCC_BCC_HARDOP_H */
