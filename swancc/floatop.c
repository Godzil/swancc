/* floatop.c - software operations on floats and doubles for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#include <swancc.h>
#include <swancc/gencode.h>
#include <swancc/reg.h>
#include <swancc/sc.h>
#include <swancc/scan.h>
#include <swancc/type.h>
#include <swancc/table.h>
#include <swancc/genloads.h>
#include <swancc/codefrag.h>
#include <swancc/function.h>
#include <swancc/floatop.h>
#include <swancc/output.h>

/*
 * f_indirect(target leaf)
 * make the float or double target indirect if it is not already
 * return nonzero iff the result is a temp double on the base of the stack
 */
bool_t f_indirect(struct symstruct *target)
{
    if (target->indcount == 0)
    {
        if (target->storage == CONSTANT)
        {
            {
                /* TODO: THIS IS NOT AN ACCEPTABLE WAY as system/compiler dependant */
                if (target->type->scalar & FLOAT)
                {
                    float val = *target->offset.offd;
                    push(constsym(((uint16_t *)&val)[1]));
                    push(constsym(((uint16_t *)&val)[0]));
                }
                else
                {
                    push(constsym(((uint16_t *)target->offset.offd)[3]));
                    push(constsym(((uint16_t *)target->offset.offd)[2]));
                    push(constsym(((uint16_t *)target->offset.offd)[1]));
                    push(constsym(((uint16_t *)target->offset.offd)[0]));
                }
            }
        }
        else if (target->type->scalar & FLOAT)
        {
            pushlist(target->storage);    /* XXX - floatregs */
        }
        else
        {
            pushlist(doubleregs);
        }
        onstack(target);
    }
    return target->flags == TEMP && target->type->scalar & DOUBLE && target->offset.offi == sp;
}

/*
 * float1op(operation code, source leaf)
 * handles all flop unary operations except inc/dec
 * result is double on stack (or in condition codes for EQOP)
 */
void float1op(op_t op, struct symstruct *source)
{
    saveopreg();
    pointat(source);
    if ((op_t)op == NEGOP)
    {
        call("Fneg");
    }
    else
    {            /* op == EQOP */
        call("Ftst");
    }
    outntypechar(source->type);
    if ((op_t)op != EQOP)
    {
        justpushed(source);
    }
    restoreopreg();
}

/*
 * floatop(operation code, source leaf, target leaf)
 * handles all flop binary operations
 * result is double on stack (or in condition codes for EQOP)
 */
void floatop(op_t op, struct symstruct *source, struct symstruct *target)
{
    store_t regmark;
    bool_t sflag;

    regmark = reguse;
    saveopreg();
    (void)f_indirect(source);
    if (!(reguse & OPREG) && (source->storage == OPREG))
    {
        reguse |= source->storage;
        saveopreg();
    }
    fpush(target);
    sflag = TRUE;
    if ( (source->flags != TEMP) || (source->offset.offi != (sp + (offset_T)DOUBLE_TYPE_SIZE)))
    {
        sflag = FALSE;
        if (source->storage == OPREG)
        {
            restoreopreg();
        }
        pointat(source);
    }

    switch (op)
    {
        case ADDOP:
            call("Fadd");
            break;
        case DIVOP:
            call("Fdiv");
            break;
        case EQOP:
            call("Fcomp");
            sp += DOUBLE_TYPE_SIZE;    /* target is popped */
            break;            /* target symbol now invalid but is not used */
        case MULOP:
            call("Fmul");
            break;
        case SUBOP:
            call("Fsub");
            break;
    }
    if (sflag)
    {
        outnl();
        sp += DOUBLE_TYPE_SIZE;    /* source is popped */
    }
    else
    {
        outntypechar(source->type);
    }
    onstack(target);
    reguse = regmark;        /* early so opreg is not reloaded if source */
    restoreopreg();
}

/*
 * fpush(source leaf of scalar type)
 * converts source to double and pushes it to stack
 * OPREG must be free
 */

void fpush(struct symstruct *source)
{
    scalar_t scalar;

    if ((scalar = source->type->scalar) & RSCALAR)
    {
        if (f_indirect(source))
        {
            return;
        }
        pointat(source);
    }
    else if (scalar & DLONG)
    {
        load(source, OPREG);
    }
    else
    {
        load(source, DREG);
    }
    call("Fpush");
    if (scalar & UNSIGNED)
    {
        outbyte('u');
    }
    outntypechar(source->type);
    justpushed(source);
}

/*
 * justpushed(target leaf)
 * records that target has just been pushed to a double on the stack
 */

void justpushed(struct symstruct *target)
{
    sp -= DOUBLE_TYPE_SIZE;
    onstack(target);
    target->type = dtype;
}
