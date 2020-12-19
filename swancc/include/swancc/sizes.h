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

extern uvalue_t intmaskto;   /* mask for ints */
extern uvalue_t maxintto;    /* maximum int */
extern uvalue_t maxlongto;   /* maximum long */
extern uvalue_t maxoffsetto; /* maximum offset */
extern uvalue_t maxshortto;  /* maximum short */
extern uvalue_t maxuintto;   /* maximum unsigned */
extern uvalue_t maxushortto; /* maximum uint16_t */
extern uvalue_t shortmaskto; /* mask for shorts */

#endif /* _SWANCC_SWANCC_SIZES_H */
