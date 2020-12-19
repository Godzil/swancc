/* align.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_ASSIGN_H
#define _SWANCC_SWANCC_ASSIGN_H

void assign(struct symstruct *source, struct symstruct *target);
void cast(struct typestruct *type, struct symstruct *target);
void extend(struct symstruct *target);

#endif /* _SWANCC_SWANCC_ASSIGN_H */
