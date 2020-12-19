/* swancc-cc1.c - "pass 1" for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#include <swancc.h>
#include <swancc/table.h>
#include <swancc/exptree.h>
#include <swancc/preproc.h>
#include <swancc/input.h>
#include <swancc/gencode.h>
#include <swancc/type.h>
#include <swancc/declare.h>
#include <swancc/output.h>

int main(int argc, char *argv[])
{
    growheap(0);        /* init order is important */
    syminit();
    etreeinit();
    ifinit();
    predefine();
    openio(argc, argv);
    codeinit();
    typeinit();
    program();
    finishup();

    /* TODO: Should NOT exit from some random place. */
    /* NOTREACHED */
    return 0;
}
