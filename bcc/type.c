/* type.c - type and storage-class routines for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#include <bcc.h>
#include <bcc/align.h>
#include <bcc/gencode.h>        /* s.b. switches.h */
#include <bcc/sc.h>
#include <bcc/scan.h>
#include <bcc/table.h>
#include <bcc/type.h>

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
    unsigned namelength;
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

struct typestruct *iscalartotype(scalar_pt scalar)
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
    register struct typestruct *type;

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

struct typestruct *prefix(constr_pt constructor, uoffset_T size, struct typestruct *type)
{
    register struct typestruct *searchtype;

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
#ifdef I80386
    if (i386_32)
    {
        uitype->typesize = itype->typesize = ptypesize = itypesize = 4;
        dtype->alignmask = fltype->alignmask = uitype->alignmask = ltype->alignmask = ultype->alignmask = itype->alignmask = ~(uoffset_T)(
            4 - 1);
        ltype->scalar = LONG;    /* not DLONG */
        ultype->scalar = UNSIGNED | LONG;
    }
#endif
    fitype = prefix(FUNCTION, ftypesize, itype);
    pctype = pointype(ctype);
    skey0 = 1;
    vtype->constructor = VOID;
}
