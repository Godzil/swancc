/* exptree.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _BCC_BCC_EXPTREE_H
#define _BCC_BCC_EXPTREE_H

struct nodestruct *castnode(struct typestruct *type, struct nodestruct *nodeptr);
void etreeinit(void);
struct nodestruct *leafnode(struct symstruct *source);
struct nodestruct *node(op_pt t, struct nodestruct *p1, struct nodestruct *p2);

#endif /* _BCC_BCC_EXPTREE_H */
