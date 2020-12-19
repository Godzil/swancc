/* function.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_FUNCTION_H
#define _SWANCC_SWANCC_FUNCTION_H

void call(char *name);
void function(struct symstruct *source);
void ldregargs(void);
void loadretexpression(void);
void listo(struct symstruct *target, offset_T lastargsp);
void listroot(struct symstruct *target);
void popframe(void);
void reslocals(void);
void ret(void);

#endif /* _SWANCC_SWANCC_FUNCTION_H */
