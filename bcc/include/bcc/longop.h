/* longop.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _BCC_BCC_LONGOP_H
#define _BCC_BCC_LONGOP_H

void longop(op_t op, struct symstruct *source, struct symstruct *target);
void long1op(op_t op, struct symstruct *target);
void outlongendian(void);

#endif /* _BCC_BCC_LONGOP_H */
