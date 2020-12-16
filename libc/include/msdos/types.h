
/* arch/i86/include/asm/types.h - Basic Linux/MT data types. */

#ifndef __MSDOS_TYPES
#define __MSDOS_TYPES

#include <asm/types.h>

typedef __u32 off_t;
typedef __u32 time_t;
typedef __u16 mode_t;
typedef __u32 loff_t;
typedef __u32 speed_t;

typedef __u32 tcflag_t;
typedef __u8  cc_t;
typedef __u16 size_t;

typedef int   ptrdiff_t;

#endif

