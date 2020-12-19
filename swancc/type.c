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

/* Global variables */
/* default sizes and long and float sizes are hard-coded into type data */
uoffset_T ctypesize;
uoffset_T dtypesize;
uoffset_T ftypesize;
uoffset_T itypesize;
uoffset_T ptypesize;
uoffset_T stypesize;

/* basic scalar types */
struct typestruct *dtype;
struct typestruct *fltype;
struct typestruct *itype;
struct typestruct *ltype;
struct typestruct *sctype;
struct typestruct *stype;
struct typestruct *uctype;
struct typestruct *uitype;
struct typestruct *ultype;
struct typestruct *ustype;
struct typestruct *vtype;

/* working type */
struct typestruct *ctype;

/* constructed types */
struct typestruct *fitype;
struct typestruct *pctype;

/* return type of current function */
struct typestruct *returntype;




uoffset_T ctypesize = 1;
uoffset_T dtypesize = 8;
uoffset_T ftypesize = 0;
uoffset_T itypesize = 2;
uoffset_T ptypesize = 2;
uoffset_T stypesize = 2;

static char skey0;
static char skey1;

struct typestruct *addstruct(char *structname)
{
    uint32_t namelength;
    struct symstruct *symptr;
    struct typestruct *structype;

    (structype = newtype())->constructor = STRUCTU;
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
        if (charptr + (namelength = strlen(structname)) >= chartop)
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

    type = qmalloc(sizeof *type);
#ifdef TS
    ++ts_n_type;
    ts_s_type += sizeof *type;
#endif
    type->typesize =        /* (uoffset_T) */
    type->scalar =        /* (scalar_t) */
    type->constructor =    /* (constr_t) */
    type->structkey[0] = 0;
    type->nexttype = type->prevtype = type->sidetype = NULL;
    type->listtype = NULL;
    type->tname = "";
    return type;
}

void outntypechar(struct typestruct *type)
{
    outnbyte(*type->tname);
}

struct typestruct *pointype(struct typestruct *type)
{
    return prefix(POINTER, ptypesize, type);
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
    switch ((searchtype = newtype())->constructor = constructor)
    {
        case ARRAY:
            searchtype->alignmask = type->alignmask;
            break;
        case FUNCTION:
            searchtype->alignmask = ~(uoffset_T)0;
            break;
        case POINTER:
            searchtype->alignmask = ~(ptypesize - 1) | alignmask;
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
            error("unsigned only applies to integral types");
            return type;
    }
}

void typeinit()
{
    fitype = prefix(FUNCTION, ftypesize, itype);
    pctype = pointype(ctype);
    skey0 = 1;
    vtype->constructor = VOID;
}
