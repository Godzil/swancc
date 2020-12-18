/* preserve.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _BCC_BCC_PRESERVE_H
#define _BCC_BCC_PRESERVE_H

void changesp(offset_T newsp, bool_pt absflag);
void loadpres(struct symstruct *source, struct symstruct *target);
void modstk(offset_T newsp);
void pres2(struct symstruct *source, struct symstruct *target);
void preserve(struct symstruct *source);
store_pt preslval(struct symstruct *source, struct symstruct *target);
void recovlist(store_pt reglist);
void savereturn(store_pt savelist, offset_T saveoffset);

#endif /* _BCC_BCC_PRESERVE_H */
