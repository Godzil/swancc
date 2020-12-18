/* genloads.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _BCC_BCC_GENLOADS_H
#define _BCC_BCC_GENLOADS_H

void addoffset(struct symstruct *source);
void address(struct symstruct *source);
void exchange(struct symstruct *source, struct symstruct *target);
store_pt getindexreg(void);
void indexadr(struct symstruct *source, struct symstruct *target);
void indirec(struct symstruct *source);
void load(struct symstruct *source, store_pt targreg);
void loadany(struct symstruct *source);
void loadreg(struct symstruct *source, store_pt targreg);
void makelessindirect(struct symstruct *source);
void movereg(struct symstruct *source, store_pt targreg);
void onstack(struct symstruct *target);
void outadr(struct symstruct *adr);
void outcregname(store_pt reg);
void outncregname(store_pt reg);
void outnregname(store_pt reg);
void outregname(store_pt reg);
void outshortregname(store_pt reg);
void pointat(struct symstruct *target);
void poplist(store_pt reglist);
void push(struct symstruct *source);
void pushlist(store_pt reglist);
void pushreg(store_pt reg);
void storereg(store_pt sourcereg, struct symstruct *target);
void struc(struct symstruct *source, struct symstruct *target);
void transfer(struct symstruct *source, store_pt targreg);

#endif /* _BCC_BCC_GENLOADS_H */
