/* preserve.c - preserve opererands or registers in use for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#include <stdio.h>

#include <swancc.h>
#include <swancc/preserve.h>
#include <swancc/gencode.h>
#include <swancc/reg.h>
#include <swancc/type.h>
#include <swancc/codefrag.h>
#include <swancc/state.h>
#include <swancc/output.h>
#include <swancc/genloads.h>

/* change stack ptr without changing condition codes */
void changesp(offset_T newsp, bool_t absflag)
{
    if (newsp != sp || ((bool_t)absflag && switchnow != NULL))
    {
#ifdef FRAMEPOINTER
        if (newsp != framep || (!(bool_t)absflag && switchnow != NULL))
        {
            outleasp();
            if (!(bool_t)absflag && switchnow != NULL)
            {
                outswoffset(newsp);
            }
            else
            {
                outoffset(newsp - framep);
            }
            outindframereg();
            outnl();
        }
        else
        {
            regtransfer(FRAMEREG, STACKREG);
        }
        sp = newsp;
        if (framep == 0)
        {
            bugerror("no frame pointer");
        }
#else
        outleasp();
        outoffset(newsp - sp);
        outindstackreg();
        outnl();
#endif /* FRAMEPOINTER */
    }
}

/* load source to any while preserving target */
void loadpres(struct symstruct *source, struct symstruct *target)
{
    store_t regmark;

    if (target->storage & ALLDATREGS)
    {
        if (source->type->scalar & CHAR)
        {
            push(target);
            load(source, DREG);
        }
        else
        {
            load(source, getindexreg());
        }
    }
    else
    {
        regmark = reguse;
        reguse |= target->storage;
        loadany(source);
        reguse = regmark;
    }
}

/* change stack ptr */
void modstk(offset_T newsp)
{
    if (newsp != sp)
    {
#ifdef FRAMEPOINTER
        if (newsp != framep || framep == 0 || switchnow != NULL)
        {
            addconst(newsp - sp, STACKREG);
        }
        else
        {
            regtransfer(FRAMEREG, STACKREG);
        }
#else
        addconst(newsp - sp, STACKREG);
#endif
        sp = newsp;
    }
}

/* preserve target without changing source */
void pres2(struct symstruct *source, struct symstruct *target)
{
    if (target->storage & allregs)
    {
        if (target->storage & (allregs - allindregs) /* XXX */ ||
            (target->indcount == 0 && target->type->scalar & (DLONG | RSCALAR)))
        {
            push(target);    /* XXX - perhaps not float */
        }
        else if (((target->storage | reguse) & allindregs) == allindregs)
        {
            loadpres(target, source);
            push(target);
        }
        else
        {
            reguse |= target->storage;
        }
    }
}

/* preserve source */
void preserve(struct symstruct *source)
{
    if (source->storage & allregs)
    {
        if (source->storage & (allregs - allindregs) /* XXX */ ||
            ((source->storage | reguse) & allindregs) == allindregs)
        {
            push(source);
        }
        else
        {
            reguse |= source->storage;
        }
    }
}

/* preserve lvalue target without changing source or target */
store_t preslval(struct symstruct *source, struct symstruct *target)
{
    store_t regpushed;

    if (target->indcount == 0)
    {
        reguse &= ~target->storage;
    }
    else
    {
        reguse = (target->storage | reguse) & allindregs;
    }
    if (!((source->type->scalar | target->type->scalar) & (DLONG | RSCALAR)) || reguse != allindregs)
    {
        return 0;
    }        /* XXX - perhaps not float */
    reguse = source->storage | target->storage;    /* free one other than s/t */
    pushreg(regpushed = getindexreg());
    reguse = ~(store_t)regpushed & allindregs;
    return regpushed;
}

void recovlist(store_t reglist)
{
    poplist(reglist);
    reguse |= (store_t)reglist;
}

static int32_t regoffset[] = {0, 0, 0, 1, 2, 3, 0, 0, 0, 4, 5};
/* CONSTANT, BREG, ax = DREG, bx = INDREG0, si = INDREG1, di = INDREG2 */
/* LOCAL, GLOBAL, STACKREG, cx = DATREG1, dx = DATREG2 */

void savereturn(store_t savelist, offset_T saveoffset)
{
    store_t reg;
    int32_t *regoffptr;
    offset_T spoffset;

    if (savelist == 0)
    {
        return;
    }

    for (reg = 1, regoffptr = regoffset ; reg != 0 ; ++regoffptr, reg <<= 1)
    {
        if (reg & savelist)
        {
            outstore();
            spoffset = saveoffset + *regoffptr * maxregsize;
#ifdef FRAMEPOINTER
            if (switchnow != NULL)
            {
                outswoffset(spoffset);
            }
            else
            {
                outoffset(spoffset - framep);
            }
            outindframereg();
#else
            outoffset(spoffset - sp);
            outindstackreg();
#endif
            outncregname(reg);
        }
    }
}
