/* loadexp.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_LOADEXP_H
#define _SWANCC_SWANCC_LOADEXP_H

value_t constexpression(void);
void initexpression(struct typestruct *type);
struct typestruct *loadexpression(store_t targreg, struct typestruct *targtype);

#endif /* _SWANCC_SWANCC_LOADEXP_H */
