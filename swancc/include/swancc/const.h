/* const.h - constants for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#ifndef _SWANCC_SWANCC_CONST_H
#define _SWANCC_SWANCC_CONST_H

/* The default compiler type ... */
#define I8088             /* target processor is Intel 8088 */

#define SELFTYPECHECK     /* check calculated type = runtime type */

#define DEBUG             /* generate compiler-debugging code */
#define OPTIMISE          /* include optimisation code */

#define FRAMEPOINTER         /* index locals off frame ptr, not stack ptr */
#define HOLDSTRINGS          /* hold strings for dumping at end
                              * since assembler has only 1 data seg */
#define DYNAMIC_LONG_ORDER 1 /* long word order spec. at compile time */

/* switches for source machine dependencies */

#define S_ALIGNMENT (sizeof(long)) /* A little safer */

/* local style */
#define FALSE (0)
#define TRUE (!FALSE)

#endif /* _SWANCC_SWANCC_CONST_H */
