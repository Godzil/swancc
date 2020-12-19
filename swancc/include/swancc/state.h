/* state.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_STATE_H
#define _SWANCC_SWANCC_STATE_H

void compound(void);
void outswoffset(offset_T offset);
void outswstacklab(void);

#endif /* _SWANCC_SWANCC_STATE_H */
