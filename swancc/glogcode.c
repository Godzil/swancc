/* glogcode.c - generate code for logical expressions for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#include <swancc.h>
#include <swancc/condcode.h>
#include <swancc/gencode.h>
#include <swancc/glogcode.h>
#include <swancc/reg.h>
#include <swancc/scan.h>
#include <swancc/sizes.h>
#include <swancc/type.h>
#include <swancc/label.h>
#include <swancc/table.h>
#include <swancc/longop.h>
#include <swancc/floatop.h>
#include <swancc/preserve.h>
#include <swancc/assign.h>
#include <swancc/codefrag.h>
#include <swancc/genloads.h>
#include <swancc/output.h>

#define cc_signed(cc) ((cc) >= 4 && (cc) < 8)

static char oppcc[] =        /* opposite condition codes LT --> GE etc */
/*  EQ, NE, RA, RN, LT, GE, LE, GT, LO, HS, LS, HI,  indices */
{
    NE, EQ, RN, RA, GE, LT, GT, LE, HS, LO, HI, LS,
};

static char reverscc[] =    /* reverse condition codes LT --> GT etc */
{
    EQ, NE, RN, RA, GT, LE, GE, LT, HI, LS, HS, LO,
};

static char testcc[] =        /* test condition codes LS --> EQ etc */
{
    EQ, NE, RA, RN, LT, GE, LE, GT, RN, RA, EQ, NE,
};

static char unsigncc[] =    /* unsigned condition codes LT --> LO etc */
{
    EQ, NE, RA, RN, LO, HS, LS, HI, LO, HS, LS, HI,
};

static void cmplocal(struct symstruct *source, struct symstruct *target, ccode_t *pcondtrue);
static void comparecond(struct nodestruct *exp, label_no truelab, label_no falselab, bool_t nojump);
static void jumpcond(struct nodestruct *exp, label_no truelab, label_no falselab, bool_t nojump);
static void loadlogical(struct symstruct *source, label_no falselab);
static void logandcond(struct nodestruct *exp, label_no truelab, label_no falselab, bool_t nojump);
static void logorcond(struct nodestruct *exp, label_no truelab, label_no falselab, bool_t nojump);
static void reduceconst(struct symstruct *source);
static void test(struct symstruct *target, ccode_t *pcondtrue);
static void testcond(struct nodestruct *exp, label_no truelab, label_no falselab, bool_t nojump);

void cmp(struct symstruct *source, struct symstruct *target, ccode_t *pcondtrue)
{
    label_no falselab;

    cmplocal(source, target, pcondtrue);
    sbranch(oppcc[(int)*pcondtrue], falselab = getlabel());
    loadlogical(target, falselab);
}

static void cmplocal(struct symstruct *source, struct symstruct *target, ccode_t *pcondtrue)
{
    scalar_t sscalar;
    scalar_t tempscalar;
    scalar_t tscalar;

    reduceconst(source);
    reduceconst(target);
    sscalar = source->type->scalar;
    tscalar = target->type->scalar;
    if ((source->storage != CONSTANT && (target->storage == CONSTANT || (sscalar & CHAR && !(tscalar & CHAR)) ||
                                         ((sscalar & CHAR || !(tscalar & CHAR)) && source->indcount == 0 &&
                                          target->indcount != 0))) || (tscalar & DLONG && target->indcount != 0))
    {
        swapsym(target, source);
        *pcondtrue = reverscc[(int)*pcondtrue];
        tempscalar = sscalar;
        sscalar = tscalar;
        tscalar = tempscalar;
    }
    if ((sscalar & CHAR && tscalar & CHAR && (source->type != sctype || target->type != sctype)) ||
        (sscalar | tscalar) & UNSIGNED || (source->type->constructor | target->type->constructor) & (ARRAY | POINTER))
    {
        *pcondtrue = unsigncc[(int)*pcondtrue];
    }
    if (source->type->scalar & DLONG)
    {
        longop(EQOP, source, target);
        return;
    }
    if (source->type->scalar & RSCALAR)
    {
        floatop(EQOP, source, target);
        return;
    }
    if (source->storage == CONSTANT)
    {
        if (sscalar & CHAR)
        {
            if (tscalar & CHAR)
            {
                *pcondtrue = unsigncc[(int)*pcondtrue];
            }
            else
            {
                source->type = iscalartotype(sscalar);
                sscalar = source->type->scalar;
            }
        }
        if (source->offset.offv == 0)
        {
            test(target, pcondtrue);
            return;
        }
    }
    if (!(sscalar & CHAR) && tscalar & CHAR)
    {
        loadpres(target, source);
        extend(target);
    }
    if (source->indcount == 0 && source->storage != CONSTANT && (source->storage != GLOBAL))
    {
        loadpres(source, target);
    }
    loadpres(target, source);
    outcmp();
    if (source->storage == GLOBAL && source->indcount == 0 && !(target->storage & (AXREG | ALREG)))
    {
        bumplc();
    }
    movereg(source, target->storage);
}

/* nojump: NB if nonzero, is ~0 so complement is 0 */
static void comparecond(struct nodestruct *exp, label_no truelab, label_no falselab, bool_t nojump)
{
    ccode_t condtrue;
    store_t regmark;
    struct symstruct *source;
    offset_T spmark;
    struct symstruct *target;

    regmark = reguse;
    spmark = sp;
    bileaf(exp);
    target = exp->left.nodeptr->left.symptr;
    source = exp->right->left.symptr;    /* exp->right != NULL since cond */
    switch (exp->tag)
    {
        case GEOP:
            condtrue = GE;
            break;
        case GTOP:
            condtrue = GT;
            break;
        case LEOP:
            condtrue = LE;
            break;
        case EQOP:
            condtrue = EQ;
            break;
        case LTOP:
            condtrue = LT;
            break;
        case NEOP:
            condtrue = NE;
            break;
    }
    cmplocal(source, target, &condtrue);
    changesp(spmark, FALSE);
    reguse = regmark;
    if ((bool_t)nojump)
    {
        lbranch(oppcc[(int)condtrue], falselab);
    }
    else
    {
        lbranch(condtrue, truelab);
    }
}

void condop(struct nodestruct *exp)
{
    label_no exitlab;
    label_no falselab;
    struct nodestruct *falsenode;
    struct symstruct *falsesym;
    label_no truelab;
    struct nodestruct *truenode;
    struct symstruct *truesym;

    jumpcond(exp->left.nodeptr, truelab = getlabel(), falselab = getlabel(), TRUE);
    deflabel(truelab);
    makeleaf(truenode = exp->right->left.nodeptr);
    loadany(truesym = truenode->left.symptr);
    if (truesym->storage & reguse)
    {
        /* This can happen if truesym was a reg variable. */
        if (truesym->type->scalar & RSCALAR)
        {
            /* XXX - always happens for non-386 with 2 regs vars assigned. */
            bugerror("loaded float or double into used reg");
        }
        load(truesym, DREG);
    }
    falsenode = exp->right->right;
    if (/* falsenode->tag != LEAF || XXX */
        truesym->type != falsenode->left.symptr->type)
    {
        cast(truenode->nodetype == falsenode->nodetype ? truenode->nodetype : exp->nodetype, truesym);
    }
    jump(exitlab = getlabel());
    deflabel(falselab);
    makeleaf(falsenode);
    falsesym = falsenode->left.symptr;
    if (falsesym->type != truesym->type)
    {
        cast(truesym->type, falsesym);
    }
    load(falsesym, truesym->storage);
    deflabel(exitlab);
    exp->tag = LEAF;
    exp->left.symptr = truesym;
}

/* nojump: NB if nonzero, is ~0 so complement is 0 */
static void jumpcond(struct nodestruct *exp, label_no truelab, label_no falselab, bool_t nojump)
{
    switch (exp->tag)
    {
        case GEOP:
        case GTOP:
        case LEOP:
        case EQOP:
        case LTOP:
        case NEOP:
            comparecond(exp, truelab, falselab, nojump);
            break;
        case LOGANDOP:
            logandcond(exp, truelab, falselab, nojump);
            break;
        case LOGNOTOP:
            jumpcond(exp->left.nodeptr, falselab, truelab, ~nojump);
            break;
        case LOGOROP:
            logorcond(exp, truelab, falselab, nojump);
            break;
        default:
            testcond(exp, truelab, falselab, nojump);
            break;
    }
}

void jumpfalse(struct nodestruct *exp, label_no label)
{
    label_no truelab;

    jumpcond(exp, truelab = getlabel(), label, TRUE);
    deflabel(truelab);
}

void jumptrue(struct nodestruct *exp, label_no label)
{
    label_no falselab;

    jumpcond(exp, label, falselab = getlabel(), FALSE);
    deflabel(falselab);
}

static void loadlogical(struct symstruct *source, label_no falselab)
{
    label_no exitlab;
    struct symstruct *target;

    target = constsym((value_t)TRUE);
    target->type = ctype;
    loadreg(target, DREG);
    sbranch(RA, exitlab = getlabel());
    deflabel(falselab);
    target = constsym((value_t)FALSE);
    target->type = ctype;
    *source = *target;
    loadreg(source, DREG);
    outnlabel(exitlab);
}


/* nojump:  NB if nonzero, is ~0 so complement is 0 */
static void logandcond(struct nodestruct *exp, label_no truelab, label_no falselab, bool_t nojump)
{
    label_no andlab;

    andlab = getlabel();
    jumpcond(exp->left.nodeptr, andlab, falselab, FALSE);
    deflabel(andlab);
    jumpcond(exp->right, truelab, falselab, nojump);
}

void logop(struct nodestruct *exp)
{
    label_no falselab;
    struct symstruct *target;
    label_no truelab;

    jumpcond(exp, truelab = getlabel(), falselab = getlabel(), FALSE);
    deflabel(truelab);
    target = constsym(0);    /* anything, loadlogical makes B reg */
    target->type = ctype;
    loadlogical(target, falselab);
    exp->tag = LEAF;
    exp->left.symptr = target;
}

/* nojump: NB if nonzero, is ~0 so complement is 0 */
static void logorcond(struct nodestruct *exp, label_no truelab, label_no falselab, bool_t nojump)
{
    label_no orlab;

    orlab = getlabel();
    jumpcond(exp->left.nodeptr, truelab, orlab, 0);
    deflabel(orlab);
    jumpcond(exp->right, truelab, falselab, nojump);
}

static void reduceconst(struct symstruct *source)
{
    if (source->storage == CONSTANT && ischarconst(source->offset.offv) &&
        (source->type->scalar & (CHAR | SHORT | INT | DLONG)) != DLONG)
    {
        if (source->type->scalar & UNSIGNED)
        {
            source->type = uctype;
        }
        else
        {
            source->type = ctype;
        }
    }
}

static void test(struct symstruct *target, ccode_t *pcondtrue)
{
    store_t targreg;

    *pcondtrue = testcc[(int)*pcondtrue];
    if (target->type->scalar & DLONG)
    {
        long1op(EQOP, target);
        return;
    }
    if (target->type->scalar & RSCALAR)
    {
        float1op(EQOP, target);
        return;
    }
    if (target->indcount != 0 || (target->storage == LOCAL && target->offset.offi != sp))
    {
        load(target, DREG);
    }
    if (target->storage == GLOBAL)
    {
        load(target, getindexreg());
    }
    if (target->type->scalar & CHAR)
    {
        load(target, DREG);
    }
    targreg = target->storage;
    if (target->offset.offi != 0 && cc_signed(*pcondtrue))
    {
        load(target, targreg);
    }

    if (target->offset.offi == 0)
    {
        outtest();
        outregname(targreg);
        outcomma();
        outnregname(targreg);
        return;
    }
    outcmp();
    outimadj(-target->offset.offi, targreg);
}

/*
 * test expression and jump depending on NE/EQ
 * nojump: NB if nonzero, is ~0 so complement is 0
 */
static void testcond(struct nodestruct *exp, label_no truelab, label_no falselab, bool_t nojump)
{
    ccode_t condtrue;
    struct symstruct *source;

    makeleaf(exp);
    source = exp->left.symptr;
    reduceconst(source);
    if (source->storage != CONSTANT)
    {
        condtrue = NE;
        test(source, &condtrue);
        if ((bool_t)nojump)
        {
            lbranch(oppcc[(int)condtrue], falselab);
        }
        else
        {
            lbranch(condtrue, truelab);
        }
    }
    else if (source->offset.offi == 0)
    {
        if ((bool_t)nojump)
        {
            jump(falselab);
        }
    }
    else if (!(bool_t)nojump)
    {
        jump(truelab);
    }
}