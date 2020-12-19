/* byteord.h - byte order dependencies for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_BYTEORD_H
#define _SWANCC_SWANCC_BYTEORD_H

#ifdef I8088
#define INT_BIG_ENDIAN 0
#define LONG_BIG_ENDIAN 1    /* longs are back to front for Xenix */
#endif

#ifdef MC6809
#define INT_BIG_ENDIAN 1     /* byte order in words is high-low */
#define LONG_BIG_ENDIAN 1    /* byte order in longs is high-low */
#endif

#endif /* _SWANCC_SWANCC_BYTEORD_H */
