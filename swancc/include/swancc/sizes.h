/* sizes.h - target scalar type sizes for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_SIZES_H
#define _SWANCC_SWANCC_SIZES_H

/*
 * the compiler is not very portable in this area
 * it only directly supports I8088
 * it assumes
 *     sizeof(source long) >= sizeof(target long)
 *     usual register size = int
 *     long = 2 int sizes
 */

#define CHBITSTO     8      /* bits in a character */
#define CHMASKTO     0xFFU   /* mask to reduce SOURCE int to TARGET uchar */
#define INT16BITSTO  16     /* not accessed in non-16 bit case */
#define INT32BITSTO  32     /* not accessed in non-32 bit case */
#define MAXINTBITSTO 32     /* max bits in an integer (var processors) */
#define MAXSCHTO     127    /* maximum signed character */
#define MAXUCHTO     255    /* maximum unsigned character */
#define MINSCHTO     (-128) /* minimum signed character */

#define isbyteoffset(n)    ((uoffset_T) (n) - MINSCHTO <= MAXSCHTO - MINSCHTO)
#define ischarconst(n)     ((uvalue_t) (n) <= MAXUCHTO)
#define isnegbyteoffset(n) ((uvalue_t) (n) + MAXSCHTO <= MAXSCHTO - MINSCHTO)
#define isshortbranch(n)   ((uoffset_T) (n) - MINSCHTO <= MAXSCHTO - MINSCHTO)


/* Varion size limits and masks for target types */
#define MASK_TO_INT (0xFFFFL)
#define MASK_TO_INT (0xFFFFL)
#define MAX_INT (0x7FFFL)
#define MAX_LONG (0x7FFFFFFFL)
#define MAX_OFFSET (0x7FFFL)
#define MAX_SHORT (0x7FFFL)
#define MAX_UINT (0xFFFFL)
#define MAX_USHORT (0xFFFFL)
#define MASK_SHORT (0xFFFFL)

#endif /* _SWANCC_SWANCC_SIZES_H */
