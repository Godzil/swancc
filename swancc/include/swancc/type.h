/* type.h - types for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#ifndef _SWANCC_SWANCC_TYPE_H
#define _SWANCC_SWANCC_TYPE_H

#include <swancc/types.h>

/* TODO: These list of variable definition are suspicious
 * Also need to check if the types are correct.
 */

/* basic scalar types */
extern struct typestruct *dtype;
extern struct typestruct *fltype;
extern struct typestruct *itype;
extern struct typestruct *ltype;
extern struct typestruct *sctype;
extern struct typestruct *stype;
extern struct typestruct *uctype;
extern struct typestruct *uitype;
extern struct typestruct *ultype;
extern struct typestruct *ustype;
extern struct typestruct *vtype;

/* working type */
extern struct typestruct *ctype;

/* constructed types */
extern struct typestruct *fitype;
extern struct typestruct *pctype;

/* return type of current function */
extern struct typestruct *returntype;

/* Prototypes */
struct typestruct *addstruct(char *structname);
struct typestruct *iscalartotype(scalar_t scalar);
struct typestruct *newtype(void);
void outntypechar(struct typestruct *type);
struct typestruct *pointype(struct typestruct *type);
struct typestruct *prefix(constr_t constructor, uoffset_T size, struct typestruct *type);
struct typestruct *promote(struct typestruct *type);
struct typestruct *tounsigned(struct typestruct *type);
void typeinit(void);

#endif /* _SWANCC_SWANCC_TYPE_H */
