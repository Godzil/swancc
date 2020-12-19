/* align.h - memory alignment requirements for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_ALIGN_H
#define _SWANCC_SWANCC_ALIGN_H

#include <sys/types.h>

#ifndef S_ALIGNMENT
# define align(x) (x)
#else
# if defined(__STDC__) && defined(_POSIX_SOURCE)
#  define align(x) (((ssize_t) (x) + (S_ALIGNMENT-1)) & ~(S_ALIGNMENT-1))
# else
#  define align(x) ((char *) (x) + (- (char) (x) & (S_ALIGNMENT-1)))
# endif
#endif

extern uoffset_T alignmask;    /* general alignment mask */


#endif /* _SWANCC_SWANCC_ALIGN_H */
