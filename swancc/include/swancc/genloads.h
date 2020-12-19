/* genloads.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_GENLOADS_H
#define _SWANCC_SWANCC_GENLOADS_H

void addoffset(struct symstruct *source);
void address(struct symstruct *source);
void exchange(struct symstruct *source, struct symstruct *target);
store_t getindexreg(void);
void indexadr(struct symstruct *source, struct symstruct *target);
void indirec(struct symstruct *source);
void load(struct symstruct *source, store_t targreg);
void loadany(struct symstruct *source);
void loadreg(struct symstruct *source, store_t targreg);
void makelessindirect(struct symstruct *source);
void movereg(struct symstruct *source, store_t targreg);
void onstack(struct symstruct *target);
void outadr(struct symstruct *adr);
void outcregname(store_t reg);
void outncregname(store_t reg);
void outnregname(store_t reg);
void outregname(store_t reg);
void outshortregname(store_t reg);
void pointat(struct symstruct *target);
void poplist(store_t reglist);
void push(struct symstruct *source);
void pushlist(store_t reglist);
void pushreg(store_t reg);
void storereg(store_t sourcereg, struct symstruct *target);
void struc(struct symstruct *source, struct symstruct *target);
void transfer(struct symstruct *source, store_t targreg);

#endif /* _SWANCC_SWANCC_GENLOADS_H */
