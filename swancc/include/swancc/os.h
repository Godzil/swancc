/* os.h - source/target operating system dependencies forswancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _SWANCC_SWANCC_OS_H
#define _SWANCC_SWANCC_OS_H

/*
  must have unix-near-compatible creat, open, read, write and close

  source O/S's supported:
    default:
      *IX
    special:
      EDOS        (#define SOS_EDOS if required)
      MSDOS        (#define SOS_MSDOS)
  target O/S's supported:
    default:
      *IX
      MSDOS
    special:
      EDOS        (#define TOS_EDOS)
*/

/* defaults */
#define CREATPERMS 0666    /* permissions for creat */
#define EOL        10      /* source newline */
#define EOLTO      10      /* target newline */
#define DIRCHAR    '/'
#define DIRSTRING  "/"
#define isabspath(fnameptr, tempcptr) ((*(tempcptr) = *(fnameptr)) == DIRCHAR)

/* special */
#ifdef SOS_EDOS
#undef DEFAULT_INCLUDE_DIR
#define DEFAULT_INCLUDE_DIR "3"
#undef DIRCHAR
#define DIRCHAR ':'
#undef DIRSTRING
#define DIRSTRING ":"
#define AS09
#undef EOL
#define EOL 13
#undef isabspath
#define isabspath(fnameptr, tempcptr) ((*(tempcptr) = *(fnameptr)) >= '0' && *(tempcptr) <= '9' && (fnameptr)[1] == DIRCHAR)
#endif

#ifdef TOS_EDOS
#undef EOLTO
#define EOLTO 13
#endif

/* don't let names dealt with here affect anything outside this file */
#undef SOS_EDOS
#undef SOS_MSDOS

#endif /* _SWANCC_SWANCC_OS_H */
