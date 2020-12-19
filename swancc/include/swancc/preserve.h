/* preserve.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_PRESERVE_H
#define _SWANCC_SWANCC_PRESERVE_H

void changesp(offset_T newsp, bool_t absflag);
void loadpres(struct symstruct *source, struct symstruct *target);
void modstk(offset_T newsp);
void pres2(struct symstruct *source, struct symstruct *target);
void preserve(struct symstruct *source);
store_t preslval(struct symstruct *source, struct symstruct *target);
void recovlist(store_t reglist);
void savereturn(store_t savelist, offset_T saveoffset);

#endif /* _SWANCC_SWANCC_PRESERVE_H */
