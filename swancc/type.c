/* type.c - type and storage-class routines for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#include <string.h>

#include <swancc.h>
#include <swancc/align.h>
#include <swancc/gencode.h>        /* s.b. switches.h */
#include <swancc/sc.h>
#include <swancc/scan.h>
#include <swancc/table.h>
#include <swancc/type.h>
#include <swancc/output.h>
#include <stdlib.h>

/* Global variables */

/* basic scalar types */
struct typestruct *dtype; /* double */
struct typestruct *fltype; /* flaot */
struct typestruct *itype; /* int */
struct typestruct *ltype; /* long */
struct typestruct *sctype; /* ?? */
struct typestruct *stype; /* short */
struct typestruct *uctype; /* unsigned chat */
struct typestruct *uitype; /* unsigned int */
struct typestruct *ultype; /* unsigned long */
struct typestruct *ustype; /* unsigned short */
struct typestruct *vtype; /* void */

/* working type */
struct typestruct *ctype; /* char */

/* constructed types */
struct typestruct *fitype; /* function */
struct typestruct *pctype; /* pointer */

/* return type of current function */
struct typestruct *returntype;

static char skey0;
static char skey1;

struct typestruct *addstruct(char *structname)
{
    uint32_t namelength;
    struct symstruct *symptr;
    struct typestruct *structype;

    structype = newtype();
    structype->constructor = STRUCTU;
    structype->alignmask = ctype->alignmask;
    if (++skey1 == 0)
    {
        ++skey1;        /* must be nonzero or key string would end */
        ++skey0;        /* will never reach 'A' to be an identifier */
    }
    structype->structkey[0] = skey0;
    structype->structkey[1] = skey1;
    if (*structname != 0)
    {
        symptr = addlorg(structname, structype);
        symptr->storage = 0;
        symptr->flags = STRUCTNAME;
        namelength = strlen(structname);
        if ((charptr + namelength) >= chartop)
        {
            growheap(namelength + 1);
        }
#ifdef TS
        ++ts_n_structname;
        ts_s_structname += namelength + 1;
#endif
        structype->tname = strcpy(charptr, structname);
        charptr += namelength + 1;
    }
    return structype;
}

struct typestruct *iscalartotype(scalar_t scalar)
{
    if (scalar & LONG)
    {
        if (scalar & UNSIGNED)
        {
            return ultype;
        }
        return ltype;
    }
    if (scalar & UNSIGNED)
    {
        return uitype;
    }
    return itype;
}

struct typestruct *newtype()
{
    struct typestruct *type;

    type = (struct typestruct *)calloc(1, sizeof(struct typestruct *));
    if (type == NULL)
    {
        fatalerror("Memory allocation error!");
    }
#ifdef TS
    ++ts_n_type;
    ts_s_type += sizeof *type;
#endif

    return type;
}

void outntypechar(struct typestruct *type)
{
    outnbyte(*type->tname);
}

struct typestruct *pointype(struct typestruct *type)
{
    return prefix(POINTER, POINTER_TYPE_SIZE, type);
}

struct typestruct *prefix(constr_t constructor, uoffset_T size, struct typestruct *type)
{
    struct typestruct *searchtype;

    for (searchtype = type->prevtype ; searchtype != NULL ; searchtype = searchtype->sidetype)
    {
        if (searchtype->constructor == (constr_t)constructor && searchtype->typesize == size)
        {
            return searchtype;
        }
    }
    searchtype = newtype();
    searchtype->constructor = constructor;
    switch (constructor)
    {
        case ARRAY:
            searchtype->alignmask = type->alignmask;
            break;
        case FUNCTION:
            searchtype->alignmask = ~(uoffset_T)0;
            break;
        case POINTER:
            searchtype->alignmask = ~(POINTER_TYPE_SIZE - 1) | alignmask;
            break;
        case STRUCTU:
            bugerror("prefixing structure/union");
            searchtype->alignmask = alignmask;
            break;
    }
    searchtype->typesize = size;
    searchtype->nexttype = type;
    searchtype->sidetype = type->prevtype;

    return type->prevtype = searchtype;
}

struct typestruct *promote(struct typestruct *type)
{
    scalar_t scalar;

    if ((scalar = type->scalar) & (CHAR | SHORT))
    {
        if (scalar & UNSIGNED)
        {
            return uitype;
        }
        return itype;
    }
    if (scalar & FLOAT)
    {
        return dtype;
    }
    if (type->constructor & ARRAY)
    {
        return pointype(type->nexttype);
    }
    if (type->constructor & FUNCTION)
    {
        return pointype(type);
    }
    return type;
}

struct typestruct *tounsigned(struct typestruct *type)
{
    switch (type->scalar & ~(UNSIGNED | DLONG))
    {
        case CHAR:
            return uctype;
        case SHORT:
            return ustype;
        case INT:
            return uitype;
        case LONG:
            return ultype;
        default:
            error("unsigned only applies to integer types");
            return type;
    }
}

void typeinit()
{
    /* Init all basic types */
    dtype = newtype(); /* double */
    fltype = newtype(); /* flaot */
    itype = newtype(); /* int */
    ltype = newtype(); /* long */
    sctype = newtype(); /* ?? */
    stype = newtype(); /* short */
    uctype = newtype(); /* unsigned chat */
    uitype = newtype(); /* unsigned int */
    ultype = newtype(); /* unsigned long */
    ustype = newtype(); /* unsigned short */
    vtype = newtype(); /* void */
    ctype = newtype(); /* char */
    fitype = newtype(); /* function */
    pctype = newtype(); /* pointer */
    returntype = newtype();

    fitype = prefix(FUNCTION, FUNCTION_TYPE_SIZE, itype);
    pctype = pointype(ctype);
    skey0 = 1;
    vtype->constructor = VOID;
}
