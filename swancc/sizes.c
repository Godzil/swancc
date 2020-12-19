/* sizes.c - target scalar type sizes for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#include <swancc.h>
#include <swancc/sizes.h>

uvalue_t intmaskto;   /* mask for ints */
uvalue_t maxintto;    /* maximum int */
uvalue_t maxlongto;   /* maximum long */
uvalue_t maxoffsetto; /* maximum offset */
uvalue_t maxshortto;  /* maximum short */
uvalue_t maxuintto;   /* maximum uint32_t */
uvalue_t maxushortto; /* maximum uint16_t */
uvalue_t shortmaskto; /* mask for shorts */