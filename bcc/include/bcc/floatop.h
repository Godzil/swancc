/* floatop.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _BCC_BCC_FLOATOP_H
#define _BCC_BCC_FLOATOP_H

bool_pt f_indirect(struct symstruct *target);
void float1op(op_pt op, struct symstruct *source);
void floatop(op_pt op, struct symstruct *source, struct symstruct *target);
void fpush(struct symstruct *source);
void justpushed(struct symstruct *target);

#endif /* _BCC_BCC_FLOATOP_H */
