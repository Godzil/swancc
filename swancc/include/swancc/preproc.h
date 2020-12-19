/* preproc.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_PREPROC_H
#define _SWANCC_SWANCC_PREPROC_H

void blanks(void);
bool_t blanksident(void);
void checknotinif(void);
void define(void);
void definestring(char *str);
void docontrol(void);
void entermac(void);
void ifinit(void);
int ifcheck(void);
void leavemac(void);
void predefine(void);
char *savedlineptr(void);
void skipcomment(void);
void skipline(void);
void undefinestring(char *str);

#endif /* _SWANCC_SWANCC_PREPROC_H */
