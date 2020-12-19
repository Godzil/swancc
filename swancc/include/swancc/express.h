/* express.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_EXPRESS_H
#define _SWANCC_SWANCC_EXPRESS_H

struct nodestruct *assignment_exp(void);
struct nodestruct *expression(void);

#endif /* _SWANCC_SWANCC_EXPRESS_H */
