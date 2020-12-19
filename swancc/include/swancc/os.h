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

/* defaults */
#define CREATPERMS 0666    /* permissions for creat */
#define EOL        10      /* source newline */
#define EOLTO      10      /* target newline */
#define DIRCHAR    '/'
#define DIRSTRING  "/"
#define isabspath(fnameptr, tempcptr) ((*(tempcptr) = *(fnameptr)) == DIRCHAR)

#endif /* _SWANCC_SWANCC_OS_H */
