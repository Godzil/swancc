/* assign.c - assignment and casting operations for swancc
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
#include <swancc/byteord.h>
#include <swancc/gencode.h>
#include <swancc/reg.h>
#include <swancc/sc.h>
#include <swancc/sizes.h>
#include <swancc/type.h>
#include <swancc/assign.h>
#include <swancc/genloads.h>
#include <swancc/codefrag.h>
#include <swancc/table.h>
#include <swancc/function.h>
#include <swancc/output.h>
#include <swancc/preserve.h>
#include <swancc/floatop.h>

static void blockmove(struct symstruct *source, struct symstruct *target);
static void call3(char *funcname, struct symstruct *target, struct symstruct *source, uoffset_T size);
static void fconvert(struct symstruct *source, struct typestruct *type);

/* block move assumes itypesize == accregsize && BREG size == 1 */
static void blockmove(struct symstruct *source, struct symstruct *target)
{
    struct symstruct oldtarget;
    uoffset_T typesize;
    struct symstruct worksource;

    oldtarget = *target;
    if ((typesize = target->type->typesize) >= 8 * itypesize || source->indcount + target->indcount != 2)
    {
        address(source);
        address(target);
        call3("_memcpy", target, source, typesize);
    }
    else
    {
        if (source->level == OFFKLUDGELEVEL)
        {
            addoffset(source);
        }    /* else kludge is lost and offsets big */
        if (target->level == OFFKLUDGELEVEL)
        {
            addoffset(target);
        }
        worksource = *source;
        for (; typesize >= itypesize ; typesize -= itypesize)
        {
            loadreg(source, DREG);
            worksource.offset.offi += itypesize;
            *source = worksource;
            storereg(DREG, target);
            target->indcount = 1;
            target->offset.offi += accregsize;
        }
        while (typesize-- != 0)
        {
            outload();
            outregname(BREG);
            outopsep();
            outadr(source);
            worksource.offset.offi += 1;
            *source = worksource;
            storereg(BREG, target);
            target->indcount = 1;
            target->offset.offi += 1;
        }
    }
    *target = oldtarget;
}

static void call3(char *funcname, struct symstruct *target, struct symstruct *source, uoffset_T size)
{
    store_t regpushed;
    offset_T spmark;
    struct symstruct *length;

    pushlist(regpushed = reguse & ~calleemask);
    spmark = sp;
    length = constsym((value_t)size);
    length->type = uitype;
    push(length);
    push(source);
    push(target);
    call(funcname);
    outnl();
    if (regpushed)
    {
        modstk(spmark);
        recovlist(regpushed);
    }
}

static void fconvert(struct symstruct *source, struct typestruct *type)
{
    offset_T spmark;

    pushlist(reguse & OPREG);
    spmark = sp;
    pointat(source);
    if (source->type->scalar & DOUBLE)
    {
        call("dto");
    }
    else
    {
        call("fto");
    }
    if (type->scalar & UNSIGNED)
    {
        outbyte('u');
    }
    outntypechar(type);
    if (type->scalar & DLONG)
    {
        if (reguse & OPREG)
        {
            bugerror("loading long into used reg");
        }
        source->storage = OPREG;
    }
    else if (type->scalar & CHAR)
    {
        source->storage = BREG;
    }
    else
    {
        source->storage = DREG;
    }
    source->offset.offi = source->flags = source->indcount = 0;
    modstk(spmark);        /* could adjust later (load instead of pop) */
    poplist(reguse & OPREG);
}

void cast(struct typestruct *type, struct symstruct *target)
{
    scalar_t newscalar;
    uoffset_T newsize;
    scalar_t oldscalar;
    uoffset_T oldsize;
    store_t targreg;

    if (type->constructor & (ARRAY | FUNCTION) || (type->constructor & STRUCTU && target->type != type))
    {
        bugerror("botched implicit cast");
        return;
    }
    if (target->type == type)
    {
        return;
    }
    if (target->type->constructor == ARRAY)
    {
        oldsize = ptypesize;
    }
    else
    {
        oldsize = target->type->typesize;
    }
    newscalar = type->scalar;
    oldscalar = target->type->scalar;
    if ((newsize = type->typesize) == oldsize && !((newscalar | oldscalar) & RSCALAR))
    {
    }
    else if (newsize == ctypesize)    /* char only */
    {
        if (oldscalar & RSCALAR)
        {
            fconvert(target, type);
        }
        else if (target->indcount == 1)
        {
#if DYNAMIC_LONG_ORDER
            if (long_big_endian)
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
# if INT_BIG_ENDIAN
            target->offset.offi += oldsize - ctypesize;
# else
        {
            if (oldscalar & DLONG)
            {
                target->offset.offi += itypesize;
            }    /* discard msword */
        }
# endif
#endif
#if DYNAMIC_LONG_ORDER
            else
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
# if INT_BIG_ENDIAN
            target->offset.offi += ctypesize;
# else
            {
            }
# endif
#endif
        }
        else if (target->storage != CONSTANT)
        {
            load(target, DREG);
            target->storage = BREG;
        }
    }
    else if ((newscalar & (SHORT | INT | LONG) && !(newscalar & DLONG)) || type->constructor & POINTER)
    {
        if (oldscalar & RSCALAR)
        {
            fconvert(target, type);
        }
        else if (oldsize < newsize)
        {
            extend(target);
        }
        else if (target->indcount == 1)
        {
#if DYNAMIC_LONG_ORDER
            if (long_big_endian)
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
            {
                if (oldscalar & DLONG)
                {
                    target->offset.offi += itypesize;
                }    /* discard msword */
            }
#endif
        }
        else
        {
            load(target, DREG);
        }
    }
    else if (newscalar & DLONG)
    {
        if (oldscalar & RSCALAR)
        {
            fconvert(target, type);
        }
        else if (target->storage != CONSTANT)
        {
            extend(target);
            load(target, DREG);
            target->storage = targreg = getindexreg();
            if (oldscalar & (UNSIGNED | CHAR) || target->type->constructor & (ARRAY | FUNCTION | POINTER))
            {
                uitol(targreg);
            }
            else
            {
                itol(targreg);
            }
        }
    }
    else if (newscalar & RSCALAR)
    {
        saveopreg();        /* XXX */
        if (oldscalar & (ISCALAR | FLOAT))
        {
            fpush(target);
        }
        if (newscalar & FLOAT)
        {
            pointat(target);
            call("dto");
            outntypechar(type);
            justpushed(target);    /* XXX - sets dtype wrong (harmless) and
                 * wastes (dtypesize - ftypesize) stack */
        }
        restoreopreg();
    }
    target->type = type;
}

    void assign(struct symstruct *source, struct symstruct *target)
{
    store_t regpushed;
    store_t sourcereg;
    scalar_t tscalar;

    if (target->type->constructor & (ARRAY | FUNCTION) || target->flags == TEMP ||
        target->flags == (LABELLED | STRING) || (target->indcount == 0 && target->flags != REGVAR))
    {
        bugerror("botched lvalue");
        return;
    }
    if (source->storage != target->storage || source->indcount != target->indcount || source->level != target->level ||
        source->offset.offi != target->offset.offi   /* kludge union cmp */
        || source->type != target->type || ((source->storage & (LOCAL | GLOBAL) || source->level == OFFKLUDGELEVEL) &&
                                            ((source->flags ^ target->flags) & LABELLED ||
                                             (source->flags & LABELLED && source->name.label != target->name.label) ||
                                             (!(source->flags & LABELLED) &&
                                              strcmp(source->name.namep, target->name.namep) != 0))))
    {
        tscalar = target->type->scalar;
        if (tscalar & CHAR && source->storage == CONSTANT)
        {
            source->offset.offv &= CHMASKTO;
            source->type = target->type;
        }
        regpushed = preslval(source, target);
        if (!(tscalar & CHAR) || source->flags != TEMP || source->offset.offi != sp ||
            source->type->typesize > itypesize)
        {
            cast(target->type, source);
        }
        if (tscalar & RSCALAR)
        {
            if ((source->storage == CONSTANT) && (!(reguse & doubleregs)))
            {
                load(source, doubleregs & ~DREG);
            }
            if ((source->storage != CONSTANT) && source->indcount == 0)
            {
                /* XXX - 386 only */
                storereg(DREG, target);
#ifdef I80386
                if (i386_32)
                {
                    if (tscalar & DOUBLE)
                    {
                        target->indcount = 1;  /* XXX outnnadr clobbers this */
                        target->offset.offi += accregsize;
                        storereg(doubleregs & ~DREG, target);
                    }
                }
                else
#endif
                if (tscalar & DOUBLE)
                {
                    uint32_t i;
                    for (i = 1 ; i ; i <<= 1)
                    {
                        if ( (i != DREG) && (doubleregs & i) )
                        {
                            target->indcount = 1;  /* XXX outnnadr clobbers this */
                            target->offset.offi += accregsize;
                            storereg(i, target);
                        }
                    }
                }
#ifdef I8088
                else if (tscalar & FLOAT)
                {
                    target->indcount = 1;  /* XXX outnnadr clobbers this */
                    target->offset.offi += accregsize;
                    storereg(DATREG2, target);
                }
#endif
                target->storage = source->storage;
                target->offset.offi = 0;
            }
            else if (f_indirect(source) && tscalar & DOUBLE && (!(reguse & OPREG) || target->storage == OPREG))
            {
                struct symstruct temptarg;

                temptarg = *target;
                pointat(&temptarg);
                call("Fpull");
                outntypechar(temptarg.type);
                sp += dtypesize;
            }
            else
            {
                blockmove(source, target);
            }
        }
        else if (target->type->constructor & STRUCTU)
        {
            blockmove(source, target);
        }
        else
        {
            if (tscalar & CHAR)
            {
                load(source, DREG);
            }
            else if (target->indcount == 0 && target->storage & regregs)
            {
                load(source, target->storage);
            }
            else
            {
                loadany(source);
            }
            if (tscalar & DLONG)
            {
                storereg(DREG, target);
                target->indcount = 1;
                target->offset.offi += accregsize;
            }
            if ((store_t)(sourcereg = source->storage) == DREG && target->type->scalar & CHAR)
            {
                sourcereg = BREG;
            }
            storereg(sourcereg, target);
            if ((store_t)regpushed != 0)
            {
                /* DLONG */
                target->indcount = 1;
                target->offset.offi -= accregsize;
                recovlist(regpushed);
            }
            else
            {
                target->storage = sourcereg;
                target->offset.offi = 0;
                if (target->level == OFFKLUDGELEVEL)
                {
                    target->level = EXPRLEVEL;
                }
            }
        }
    }
}

/* extend char or short to int (unsigned if from unsigned) */
void extend(struct symstruct *target)
{
    scalar_t tscalar;

    if ((tscalar = target->type->scalar) & (CHAR | SHORT))
    {
        if (target->storage != CONSTANT && target->type->typesize < itypesize)
        {
            load(target, DREG);
            if (target->type == sctype)
            {
                sctoi();
            }
#ifdef I8088
                else if (tscalar & SHORT)
                {
                    if (tscalar & UNSIGNED)
                    {
                        ustoi();
                    }
                    else
                    {
                        stoi();
                    }
                }
#endif
            else
            {
                ctoi();
            }
            target->storage = DREG;
        }
        if (tscalar & UNSIGNED)
        {
            target->type = uitype;
        }
        else
        {
            target->type = itype;
        }
    }
}

