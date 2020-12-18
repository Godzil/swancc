/* gencode.c - generate code for an expression tree for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#define islvalop(op) \
    (((op) >= ASSIGNOP && (op) <= SUBABOP) || (op) == PTRADDABOP)

#include <bcc.h>
#include <bcc/align.h>
#include <bcc/condcode.h>
#include <bcc/reg.h>
#include <bcc/sc.h>
#include <bcc/scan.h>
#include <bcc/type.h>

#include <bcc/gencode.h>
#include <bcc/sizes.h>

#define FIRSTOPDATA GTOP

#if MAXINDIRECT <= 1
# define istooindirect(t) ((t)->indcount > MAXINDIRECT)
#else
# define istooindirect(t) ((t)->indcount >= MAXINDIRECT && \
               ((t)->indcount > MAXINDIRECT || \
                (t)->type->typesize > maxregsize || \
                (t)->type->constructor & FUNCTION))
#endif

#ifdef I8088
#if NOTFINISHED
store_pt allregs = BREG | DREG | DATREG1 | DATREG2 | INDREG0 | INDREG1 | INDREG2;
#else
store_pt allregs = BREG | DREG | INDREG0 | INDREG1 | INDREG2;
#endif
store_pt allindregs = INDREG0 | INDREG1 | INDREG2;
uoffset_T alignmask = ~(uoffset_T)0x0001;
bool_t arg1inreg = FALSE;
store_pt calleemask = INDREG1 | INDREG2;
bool_t callersaves = FALSE;
char *callstring = "call\t";
store_pt doubleargregs = DREG | INDREG0 | DATREG1 | DATREG2;
store_pt doubleregs = DREG | INDREG0 | DATREG1 | DATREG2;
store_pt doublreturnregs = DREG | INDREG0 | DATREG1 | DATREG2;
offset_T jcclonger = 3;
offset_T jmplonger = 1;
char *jumpstring = "br \t";
char *regpulllist = "f2ax2ax2bx2si2di2bp2qx2qx2cx2dx2";
char *regpushlist = "dx2cx2qx2qx2bp2di2si2bx2ax2ax2f2";
#if NOTFINISHED
store_pt regregs = INDREG1 | INDREG2 | DATREG1 | DATREG2;
#else
store_pt regregs = INDREG1 | INDREG2;
#endif

char *acclostr = "al";
char *accumstr = "ax";
char *badregstr = "qx";
char *dreg1str = "cx";
char *dreg1bstr = "cl";
char *dreg2str = "dx";
char *ireg0str = "bx";
char *ireg1str = "si";
char *ireg2str = "di";
#ifdef FRAMEPOINTER
char *localregstr = "bp";
#else
char *localregstr = "sp";
#endif
char *stackregstr = "sp";
#endif

#ifdef MC6809
store_pt allregs = BREG | DREG | INDREG0 | INDREG1 | INDREG2;
store_pt allindregs = INDREG0 | INDREG1 | INDREG2;
uoffset_T alignmask = ~(uoffset_T) 0x0000;
bool_t arg1inreg = TRUE;
store_pt calleemask = INDREG1 | INDREG2;
bool_t callersaves = TRUE;
char *callstring = "JSR\t>";
store_pt doubleargregs = DREG | INDREG0 | INDREG1 | INDREG2;
store_pt doubleregs = DREG | INDREG0 | INDREG1 | INDREG2;
store_pt doublreturnregs = DREG | INDREG0 | INDREG1 | INDREG2;
offset_T jcclonger = 2;
offset_T jmplonger = 1;
char *jumpstring = "JMP\t>";
char *regpulllist = "CC1B1D2X2U2Y2DP1PC2";
char *regpushlist = "PC2DP1Y2U2X2D2B1CC1";
store_pt regregs = INDREG1 | INDREG2;

char *acclostr = "B";
char *accumstr = "D";
char *badregstr = "Q";
char *ireg0str = "X";
char *ireg1str = "U";
char *ireg2str = "Y";
char *localregstr = "S";
#endif

uoffset_T accregsize = 2;
#ifdef FRAMEPOINTER
uoffset_T frameregsize = 2;
#endif
uoffset_T maxregsize = 2;
uoffset_T opregsize = 2;
uoffset_T pshregsize = 2;
uoffset_T returnadrsize = 2;

#ifndef MC6809
uvalue_t intmaskto = 0xFFFFL;
uvalue_t maxintto = 0x7FFFL;
uvalue_t maxlongto = 0x7FFFFFFFL;
uvalue_t maxoffsetto = 0x7FFFL;
uvalue_t maxshortto = 0x7FFFL;
uvalue_t maxuintto = 0xFFFFL;
uvalue_t maxushortto = 0xFFFFL;
uvalue_t shortmaskto = 0xFFFFL;
#endif

static store_pt callermask;
static offset_T lastargsp;

static smalin_t opdata[] = {
    /*    GTOP, LTOP, ADDOP, DIVOP, */
    GT, LT, 0, 0,
    /*    MODOP, LOGNOTOP, NOTOP, STRUCELTOP, */
    0, 0, 0, 0,
    /*    STRUCPTROP, ASSIGNOP, ADDABOP, ANDABOP, */
    0, 0, 0, ANDOP,
    /*    DIVABOP, EORABOP, MODABOP, MULABOP, */
    DIVOP, EOROP, MODOP, MULOP,
    /*    ORABOP, SLABOP, SRABOP, SUBABOP, */
    OROP, SLOP, SROP, 0,
    /*    COMMAOP, COLONOP, LOGOROP, LOGANDOP, */
    0, 0, 0, 0,
    /*    EQOP, NEOP, GEOP, LEOP, */
    EQ, NE, GE, LE,
};

static void abop(op_pt op, struct symstruct *source, struct symstruct *target);

static void smakeleaf(struct nodestruct *exp);

static void tcheck(struct nodestruct *exp);

static void abop(op_pt op, struct symstruct *source, struct symstruct *target)
{
    store_pt regmark;
    store_pt regpushed;
    store_pt regtemp;
    struct symstruct temptarg;

    regpushed = preslval(source, target);
    temptarg = *target;
    if ((source->type->scalar ^ target->type->scalar) & (DLONG | RSCALAR) && op != SLABOP &&
        op != SRABOP)    /* XXX - perhaps not float */
    {
        pres2(target, source);
        cast(source->type, &temptarg);
    }
    switch (op)
    {
        case ADDABOP:
            add(source, &temptarg);
            break;
        case ANDABOP:
        case EORABOP:
        case ORABOP:
            op1((opdata - FIRSTOPDATA)[op], source, &temptarg);
            break;
        case DIVABOP:
        case MODABOP:
        case MULABOP:
        case SLABOP:
        case SRABOP:
            softop((opdata - FIRSTOPDATA)[op], source, &temptarg);
            break;
        case PTRADDABOP:
            regtemp = 0;
            if ((reguse & allindregs) == allindregs)
            {
                /* free a temporary index not used for source or target */

                regmark = reguse;
                reguse = source->storage | temptarg.storage;
                pushreg(regtemp = getindexreg());
                reguse = regmark & ~regtemp;
            }
            indexadr(source, &temptarg);
            if (regtemp)
            {
                load(&temptarg, DREG);
                recovlist(regtemp);
            }
            break;
        case SUBABOP:
            sub(source, &temptarg);
            break;
    }
    assign(&temptarg, target);
    recovlist(regpushed);
}

void bileaf(struct nodestruct *exp)
{
    bool_t commutop;
    bool_t tookaddress;
    store_t regmark;
    struct nodestruct *indchase;
    struct nodestruct *left;
    struct nodestruct *right;
    struct symstruct *source;
    struct symstruct *target;

    left = exp->left.nodeptr;
    if ((right = exp->right) == NULL)
    {
        makeleaf(left);
#ifdef DEBUG
        debug(exp);
#endif
        return;
    }
    switch (exp->tag)
    {
        case ADDOP:
        case ANDOP:
        case EOROP:
        case OROP:
        case EQOP:
        case NEOP:
        case MULOP:
            commutop = TRUE;
            break;
        case FUNCOP:
            makeleaf(left);
            if ((target = left->left.symptr)->storage & allregs && right->tag != LEAF && target->flags != REGVAR)
            {
                if (target->indcount == 0)
                {
                    push(target);
                }
                else
                {
                    --target->indcount;
                    push(target);
                    ++target->indcount;
                }
            }
        default:
            commutop = FALSE;
            break;
    }
    regmark = reguse;
    if (right->tag != LEAF)
    {
        if (left->tag != LEAF && commutop && left->weight > right->weight)
        {
            exp->left.nodeptr = right;
            right = exp->right = left;
            left = exp->left.nodeptr;
#ifdef DEBUG
            debugswap();
#endif
        }
        makeleaf(right);
    }
    else if (left->tag != LEAF)
    {
        makeleaf(left);
    }
    source = right->left.symptr;
    if (left->tag != LEAF)
    {
        for (indchase = left ;
             indchase->tag == INDIRECTOP || indchase->tag == STRUCELTOP ; indchase = indchase->left.nodeptr)
        {
        }
        tookaddress = FALSE;
        if (source->storage & allindregs || indchase->tag != LEAF)
        {
            if (exp->nodetype->constructor & STRUCTU && exp->tag == ASSIGNOP)
            {
                address(source);
                tookaddress = TRUE;
            }
            if (source->storage & allindregs && source->indcount == 0 &&
                (source->type->scalar & (DLONG | RSCALAR) || (left->tag == FUNCOP && source->flags != REGVAR)))
            {
                push(source);    /* XXX - perhaps not float */
            }
            else
            {
                preserve(source);
            }
        }
        makeleaf(left);
        if (tookaddress)
        {
            indirec(source);
        }
    }
    target = left->left.symptr;
    if (istooindirect(source))
    {
        /*
         * want to makelessindirect(source)
         * this uses source->storage if that is a free index
         * otherwise, must preserve target if that is an index
         */

        tookaddress = FALSE;
        if (!(source->storage & ~reguse & allindregs) && target->storage & allindregs)
        {
            /*
             * want to pres2(source, target)
             * this requires target to be < MAXINDIRECT indirect
             * it is safe to makelessindirect(target)
             * since source is not a free index
             */

            if (islvalop(exp->tag) && target->indcount != 0)
            {
                address(target);
                tookaddress = TRUE;
            }
            if (istooindirect(target))
            {
                makelessindirect(target);
            }
            pres2(source, target);
        }
        makelessindirect(source);
        if (tookaddress)
        {
            indirec(target);
        }
    }
    if (istooindirect(target))
    {
        tookaddress = FALSE;
        if (!(target->storage & ~reguse & allindregs) && source->storage & allindregs)
        {
            if (exp->nodetype->constructor & STRUCTU && exp->tag == ASSIGNOP)
            {
                address(source);
                tookaddress = TRUE;
            }
            pres2(target, source);
        }
        makelessindirect(target);
        if (tookaddress)
        {
            indirec(source);
        }
    }
    reguse = regmark;
#ifdef DEBUG
    debug(exp);
#endif
    if (commutop &&
        ((target->storage == CONSTANT && !(target->type->scalar & (DLONG | RSCALAR))) || source->storage & ALLDATREGS ||
         (source->type->scalar & (DLONG | RSCALAR) && source->indcount == 0 && target->indcount != 0)))
    {
        exp->left.nodeptr = right;
        exp->right = left;
#ifdef DEBUG
        debugswap();
#endif
    }
}

fastin_pt bitcount(register uvalue_t number)
{
    register fastin_pt count;

    for (count = 0 ; number != 0 ; number >>= 1)
    {
        if (number & 1)
        {
            ++count;
        }
    }
    return count;
}

void codeinit()
{
#ifdef I80386
    if (i386_32)
    {
        /* Need DATREG2 for doubles although handling of extra data regs is
         * not finished.
         * XXX - might need more regs for 16-bit mode doubles or floats.
         */
        allregs = BREG | DREG | INDREG0 | INDREG1 | INDREG2 | DATREG1 | DATREG1B | DATREG2;
#if NOTFINISHED
        allindregs = INDREG0 | INDREG1 | INDREG2 | DATREG1 | DATREG2;
#else
        allindregs = INDREG0 | INDREG1 | INDREG2;
#endif
        alignmask = ~(uoffset_T)0x00000003;
        calleemask = INDREG0 | INDREG1 | INDREG2;
        doubleargregs = DREG | DATREG2;
        doubleregs = DREG | DATREG2;
        doublreturnregs = DREG | DATREG2;
        jcclonger = 4;
        jmplonger = 3;
        regpulllist = "fd4eax4eax4ebx4esi4edi4ebp4qx4qx4ecx4edx4";
        regpushlist = "edx4ecx4qx4qx4ebp4edi4esi4ebx4eax4eax4fd4";

        accumstr = "eax";
        dreg1str = "ecx";
        dreg2str = "edx";
        ireg0str = "ebx";
        ireg1str = "esi";
        ireg2str = "edi";
#ifdef FRAMEPOINTER
        localregstr = "ebp";
#else
        localregstr = "esp";
#endif
        stackregstr = "esp";

        opregsize = returnadrsize = pshregsize = maxregsize =
        #ifdef FRAMEPOINTER
            frameregsize =
        #endif
            accregsize = 4;

        intmaskto = (unsigned long)0xFFFFFFFFL;
        maxintto = 0x7FFFFFFFL;
        maxoffsetto = 0x7FFFFFFFL;
        maxuintto = (unsigned long)0xFFFFFFFFL;
    }
#endif
#ifdef POSINDEPENDENT
    if (posindependent)
    {
# ifdef MC6809
    callstring = "LBSR\t";
    jumpstring = "LBRA\t";
# endif
    }
#endif
    if (callersaves)
    {
        calleemask = 0;
    }
    callermask = ~calleemask;
#ifdef FRAMEPOINTER
    funcsaveregsize = bitcount((uvalue_t)calleemask) * maxregsize + frameregsize;
    funcdsaveregsize = bitcount((uvalue_t)calleemask & ~doubleregs) * maxregsize + frameregsize;
    framelist = FRAMEREG | calleemask;
#else
    funcsaveregsize = bitcount((uvalue_t) calleemask) * maxregsize;
    funcdsaveregsize = bitcount((uvalue_t) calleemask & ~doubleregs)
               * maxregsize;
#endif
}

fastin_pt highbit(register uvalue_t number)
{
    register fastin_pt bit;

    for (bit = -1 ; number != 0 ; number >>= 1)
    {
        ++bit;
    }
    return bit;
}

void makeleaf(struct nodestruct *exp)
{
    ccode_t condtrue;
    op_pt op;
    store_t regmark;
    offset_T saveargsp = 0; /* for -Wall */
    store_t savelist = 0; /* for -Wall */
    offset_T saveoffset = 0; /* for -Wall */
    struct symstruct *source;
    offset_T spmark;
    struct symstruct *structarg = 0; /* for -Wall */
    struct symstruct *target;

    if ((op_t)(op = exp->tag) == LEAF)
    {
        target = exp->left.symptr;
        if (istooindirect(target))
        {
            makelessindirect(target);
        }
#ifdef SELFTYPECHECK
        tcheck(exp);
#endif
        return;
    }
    if ((op_t)op == INDIRECTOP || (op_t)op == STRUCELTOP)
    {
        smakeleaf(exp);
        target = exp->left.symptr;
        if (istooindirect(target))
        {
            makelessindirect(target);
        }
#ifdef SELFTYPECHECK
        tcheck(exp);
#endif
        return;
    }
    if ((op_t)op == COMMAOP)
    {
        spmark = sp;
        makeleaf(exp->left.nodeptr);
        modstk(spmark);
        makeleaf(exp->right);
        exp->tag = LEAF;
        exp->left.symptr = exp->right->left.symptr;
#ifdef SELFTYPECHECK
        tcheck(exp);
#endif
        return;
    }
    if ((op_t)op == CONDOP)
    {
        condop(exp);
#ifdef SELFTYPECHECK
        tcheck(exp);
#endif
        return;
    }
    if ((op_t)op == LOGANDOP || (op_t)op == LOGNOTOP || (op_t)op == LOGOROP)
    {
        logop(exp);
#ifdef SELFTYPECHECK
        tcheck(exp);
#endif
        return;
    }
    regmark = reguse;
    if ((op_t)op == FUNCOP)
    {
        saveargsp = lastargsp;
        lastargsp = savelist = 0;
        if (exp->nodetype->constructor & STRUCTU)
        {
            modstk(sp - (offset_T)exp->nodetype->typesize);
            onstack(structarg = constsym((value_t)0));
        }
        else
        {
            if (exp->nodetype->scalar & DOUBLE)
            {
                if (regmark & doublreturnregs)
                {
                    savelist = doublreturnregs;
                }
            }
            else if (regmark & RETURNREG)
            {    /* XXX size long == float ? */
                savelist = exp->nodetype->scalar & DLONG ? LONGRETURNREGS : RETURNREG;
            }
            if (savelist != 0)
            {
                modstk(saveoffset = sp - exp->nodetype->typesize);
            }
        }
        pushlist(regmark & callermask);
    }
    spmark = sp;
    bileaf(exp);
    if (exp->right != NULL)
    {
        source = exp->right->left.symptr;
    }
    else
    {
        source = NULL;
    }
    target = exp->left.nodeptr->left.symptr;
    switch ((op_t)op)
    {
        case ADDABOP:
        case ANDABOP:
        case DIVABOP:
        case EORABOP:
        case SUBABOP:
        case MODABOP:
        case MULABOP:
        case ORABOP:
        case PTRADDABOP:
        case SLABOP:
        case SRABOP:
            abop(op, source, target);
            break;
        case ADDOP:
            add(source, target);
            break;
        case ADDRESSOP:
            address(target);
            break;
        case ANDOP:
        case EOROP:
        case OROP:
            op1(op, source, target);
            break;
        case ASSIGNOP:
            assign(source, target);
            break;
        case CASTOP:
            cast(source->type, target);
            break;
        case DIVOP:
        case MODOP:
        case MULOP:
        case SLOP:
        case SROP:
            softop(op, source, target);
            break;
        case EQOP:
        case GEOP:
        case GTOP:
        case LEOP:
        case LTOP:
        case NEOP:
            condtrue = (opdata - FIRSTOPDATA)[op];
            cmp(source, target, &condtrue);
            break;
        case FUNCOP:
            /*
             * kludge update pushed regs
             * may only work for si, di
             * -2 skips for ax and bx
             * need dirtymask to mostly avoid this
             */
            savereturn(regmark & callermask & regregs, spmark - 2 * (offset_T)pshregsize);
            if (exp->nodetype->constructor & STRUCTU)
            {
                address(structarg);
                push(structarg);
            }
            function(target);
            break;
        case INDIRECTOP:
            indirec(target);
            break;
        case LISTOP:
            listo(target, lastargsp);
            lastargsp = sp;
            break;
        case NEGOP:
            neg(target);
            break;
        case NOTOP:
            not(target);
            break;
        case POSTDECOP:
        case POSTINCOP:
        case PREDECOP:
        case PREINCOP:
            incdec(op, target);
            break;
        case PTRADDOP:
            indexadr(source, target);
            break;
        case PTRSUBOP:
            ptrsub(source, target);
            break;
        case ROOTLISTOP:
            listroot(target);
            lastargsp = sp;
            break;
        case STRUCELTOP:
            struc(source, target);
            break;
        case SUBOP:
            sub(source, target);
            break;
    }
    if (target->storage == LOCAL && target->offset.offi < spmark && target->flags == TEMP)
    {
        spmark = target->offset.offi;
    }
#if 1 /* XXX - why does sp get changed without this? */
    if ((op_t)op != ROOTLISTOP)
#endif
    {
        modstk(spmark);
    }
    if ((op_t)op == FUNCOP)
    {
        lastargsp = saveargsp;
        if (savelist != 0)
        {
            savereturn(savelist, saveoffset);
            onstack(target);
            target->offset.offi = saveoffset;
        }
        recovlist(regmark & callermask);
    }
    reguse = regmark;
    exp->tag = LEAF;
    exp->left.symptr = target;
    if (istooindirect(target))
    {
        makelessindirect(target);
    }
#ifdef SELFTYPECHECK
    tcheck(exp);
#endif
}

static void smakeleaf(struct nodestruct *exp)
{
    struct nodestruct *left;

    left = exp->left.nodeptr;
    if (left->tag == INDIRECTOP || left->tag == STRUCELTOP)
    {
        smakeleaf(left);
    }
    else if (left->tag != LEAF)
    {
        makeleaf(left);
    }
    if (exp->tag == INDIRECTOP)
    {
        indirec(left->left.symptr);
    }
    else
    {
        if (left->left.symptr->indcount > MAXINDIRECT + 1)
        {
            makelessindirect(left->left.symptr);
        }
        struc(exp->right->left.symptr, left->left.symptr);
    }
    exp->tag = LEAF;
    exp->left.symptr = left->left.symptr;
}

#ifdef SELFTYPECHECK

static void tcheck(register struct nodestruct *exp)
{
    register struct symstruct *target;

    if (exp->nodetype != (target = exp->left.symptr)->type)
    {
        {
            bugerror("botched nodetype calculation");
#ifdef DEBUG
            comment();
            outstr("runtime type is ");
            dbtype(target->type);
            outstr(", calculated type is ");
            dbtype(exp->nodetype);
            outnl();
#endif
        }
    }
}

#endif /* SELFTYPECHECK */
