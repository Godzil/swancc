/* function.c - function call protocol for swancc
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
#include <swancc/function.h>
#include <swancc/align.h>
#include <swancc/byteord.h>
#include <swancc/gencode.h>
#include <swancc/parser.h>
#include <swancc/reg.h>
#include <swancc/sc.h>
#include <swancc/table.h>
#include <swancc/type.h>
#include <swancc/scan.h>
#include <swancc/output.h>
#include <swancc/codefrag.h>
#include <swancc/genloads.h>
#include <swancc/express.h>
#include <swancc/assign.h>
#include <swancc/loadexp.h>
#include <swancc/preserve.h>
#include <swancc/debug.h>
#include <swancc/label.h>

#define ADJUSTLONGRETURN
#define CANHANDLENOFRAME
#undef  CANHANDLENOFRAME
#define STUPIDFRAME

static void out_callstring(void);

/* call a named (assembly interface) procedure, don't print newline after */
void call(char *name)
{
    out_callstring();
    outstr(name);
}

void function(struct symstruct *source)
{
    if (source->indcount == 0 && source->storage == GLOBAL && !(source->flags & LABELLED) && *source->name.namep != 0)
    {
        out_callstring();
        outnccname(source->name.namep);
    }
    else
    {
        outcalladr();
        outadr(source);
    }
    source->type = source->type->nexttype;
#ifdef LONGRETSPECIAL /* LONGRETURNREGS!=RETURNREG && RETURNREG==LONGREG2 */
    if (source->type->scalar & DLONG)
    {
#ifdef ADJUSTLONGRETURN
#if DYNAMIC_LONG_ORDER
        if (long_big_endian)
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
        {
            regexchange(LONGREG2, LONGRETURNREGS & ~LONGREG2);
            regexchange(LONGREG2, DXREG);
        }
#endif
#if DYNAMIC_LONG_ORDER
        else
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
        {
            regtransfer(DXREG, LONGRETURNREGS & ~LONGREG2);
        }
#endif
#endif
        source->storage = LONGRETURNREGS & ~LONGREG2;
    }
    else
#endif
    if (source->type->scalar & CHAR)
    {
#if RETURNREG != DREG
        transfer(source, DREG);
#endif
        source->storage = BREG;
    }

    if (source->type->scalar & DOUBLE)
    {
        source->storage = doublreturnregs;
    }
    else if (source->type->scalar & FLOAT)
    {
        source->storage = RETURNREG | DATREG2;
    }
    else
    {
        source->storage = RETURNREG;
    }

    source->offset.offi = source->indcount = 0;
    if (source->level == OFFKLUDGELEVEL)
    {
        source->level = EXPRLEVEL;
    }
    if (source->type->constructor & STRUCTU)
    {
        transfer(source, getindexreg());    /* so it can be indirected
                     * and/or preserved in blockmove() */
        source->indcount = 1;
        source->flags = TEMP;    /* kludge so blockpush can be avoided */
    }
}

void ldregargs()
{
    struct symstruct *symptr;
    store_t targreg;
    struct symstruct temptarg;

    for (symptr = &locsyms[0] ; symptr < locptr && symptr->level == ARGLEVEL ; symptr = (struct symstruct *)align(
        &symptr->name.namea[strlen(symptr->name.namea) + 1]))
    {
        if ((store_t)(targreg = symptr->storage) & allregs)
        {
            /* load() is designed to work on expression symbols, so don't
             * trust it on reg variables although it almost works.
             */
            temptarg = *symptr;
            if (arg1inreg && symptr == &locsyms[0])
            {
                temptarg.storage = ARGREG;
                temptarg.offset.offi = 0;
            }
            else
            {
                temptarg.storage = LOCAL;
                temptarg.indcount = 1;
            }
            load(&temptarg, targreg);
            symptr->offset.offi = 0;
        }
    }
    regarg = FALSE;
}

void loadretexpression()
{
    if (returntype->constructor & STRUCTU)
    {
        struct nodestruct *etmark;
        struct nodestruct *exp;
        struct symstruct *exprmark;
        struct symstruct *structarg;

        etmark = etptr;
        exprmark = exprptr;
        exp = expression();
        makeleaf(exp);
        structarg = constsym((value_t)0);
        structarg->type = pointype(returntype);
        onstack(structarg);
        indirec(structarg);
        structarg->flags = 0;    /* assign() doesn't like TEMP even for indir */
        structarg->offset.offi = returnadrsize;
        assign(exp->left.symptr, structarg);
        etptr = etmark;
        exprptr = exprmark;
    }
#ifdef LONGRETSPECIAL /* LONGRETURNREGS!=RETURNREG && RETURNREG==LONGREG2 */
    else if (returntype->scalar & DLONG)
    {
        loadexpression(LONGRETURNREGS & ~LONGREG2, returntype);
#ifdef ADJUSTLONGRETURN
#if DYNAMIC_LONG_ORDER
        if (long_big_endian)
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
        {
            regexchange(LONGREG2, DXREG);
            regexchange(LONGREG2, LONGRETURNREGS & ~LONGREG2);
        }
#endif
#if DYNAMIC_LONG_ORDER
        else
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
        {
            regtransfer(LONGRETURNREGS & ~LONGREG2, DXREG);
        }
#endif
#endif
    }
    else
#endif
    {
        if (returntype->scalar & DOUBLE)
        {
            loadexpression(doublreturnregs, returntype);
        }
        else if (returntype->scalar & FLOAT)
        {
            loadexpression(/* REURNREG|*/ DATREG2, returntype);
        }
        else
        {
            loadexpression(RETURNREG, returntype);
        }
    }
}

void listo(struct symstruct *target, offset_T lastargsp)
{
    extend(target);
    push(target);
    if (lastargsp != 0 && sp != lastargsp - target->type->typesize)
    {
        loadany(target);
        modstk(lastargsp);
        push(target);
        if (sp != lastargsp - target->type->typesize)
        {
            bugerror("botched push of arg");
#ifdef DEBUG
            outstr("arg type is ");
            dbtype(target->type);
            outnl();
#endif
        }
    }
}

void listroot(struct symstruct *target)
{
    extend(target);
    /* necessary regs are free since they were saved for function */
    if (target->type->scalar & DLONG)
    {
        load(target, LONGARGREGS & ~LONGREG2);
    }
    else
    {
        load(target, ARGREG);
    }
}

static void out_callstring()
{
    outop3str(callstring);
}

#ifdef FRAMEPOINTER

void popframe()
{
#ifdef STUPIDFRAME
    poplist(callee1mask);    /*XXX: Add if round this */
    poplist(FRAMEREG);
#else
    poplist(frame1list);
#endif
}

#endif

/*
 * reserve storage for locals if necessary
 * also push 1st function arg and load register args if necessary
 */
void reslocals()
{
#ifdef FRAMEPOINTER
#ifndef STUPIDFRAME
    bool_t loadframe = FALSE;
#endif
#endif

    if (switchnow != NULL)
    {
#ifdef FRAMEPOINTER
        if (framep == 0 && softsp != sp)
        {
            bugerror("local variables in switch statement messed up, sorry");
        }
#else
        if (sp != softsp)
            bugerror("local variables in switch statement don't work, sorry");
#endif
        if (lowsp > softsp)
        {
            lowsp = softsp;
        }
        sp = softsp;
        return;
    }
#ifdef FRAMEPOINTER
    if (framep == 0)
    {
#ifdef STUPIDFRAME
        pushreg(FRAMEREG);
        regtransfer(STACKREG, FRAMEREG);
        framep = sp;
        pushlist(callee1mask);        /*XXX: Add if round this */
#else /* not STUPIDFRAME */
#ifdef CANHANDLENOFRAME
        if (stackarg || softsp != -frameregsize)    /* args or locals */
#endif
        {
            pushlist(frame1list);
            loadframe = TRUE;
        }
#endif /* not STUPIDFRAME */
    }
#else
    if (sp == 0)
    pushlist(callee1mask);
#endif /* FRAMEPOINTER */
    if (arg1size)
    {
        switch ((int32_t)arg1size)
        {
            case 8:
                pushlist(doubleargregs);
                break;
            case 4:
                pushlist(LONGARGREGS);
                break;
            case 2:
                pushlist(ARGREG);
        }
        arg1size = 0;        /* show 1st arg allocated */
    }
#ifdef FRAMEPOINTER
#ifndef STUPIDFRAME /* else this moved above for compat with Xenix cc frame */
    if (loadframe || softsp != -frameregsize)
    modstk(softsp);
    /* else avoid modstk() because softsp holds space for frame pointer only) */
    /* but pointer has not been pushed (must keep softsp for later levels) */
    if (loadframe)
    {
    regtransfer(STACKREG, FRAMEREG);
    framep = sp;
    }
#else /* STUPIDFRAME */
    modstk(softsp);
#endif /* STUPIDFRAME */
#else /* no FRAMEPOINTER */
    modstk(softsp);
#endif /* FRAMEPOINTER */
    if (regarg)
    {
        ldregargs();
    }
}

/* clean up stack and return from a function */
void ret()
{
#ifdef FRAMEPOINTER
    offset_T newsp;

    if (framep != 0)
    {
        newsp = -(offset_T)func1saveregsize;
        if (switchnow != NULL || newsp - sp >= 0x80)
        {
            changesp(newsp, TRUE);
        }
        else
        {
            modstk(newsp);
        }
        popframe();
    }
    outreturn();
#else /* no FRAMEPOINTER */
    if (sp != 0)
    {
    modstk(-(offset_T) func1saveregsize);
    poplist(callee1mask);
    }
    outreturn();
#endif /* no FRAMEPOINTER */
}