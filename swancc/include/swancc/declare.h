/* declare.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_DECLARE_H
#define _SWANCC_SWANCC_DECLARE_H

void colon(void);
void decllist(void);
void lparen(void);
void needvarname(void);
void program(void);
void rbrace(void);
void rbracket(void);
void rparen(void);
void semicolon(void);
struct typestruct *typename (void);

#endif /* _SWANCC_SWANCC_DECLARE_H */
