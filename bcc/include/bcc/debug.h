/* debug.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _BCC_BCC_DEBUG_H
#define _BCC_BCC_DEBUG_H

void dbitem(struct symstruct *item);
void dbtype(struct typestruct *type);
void debug(struct nodestruct *exp);
void debugswap(void);

#endif /* _BCC_BCC_DEBUG_H */
