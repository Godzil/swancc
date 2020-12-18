/* debug.c - print debug messages for operators for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#include <stdio.h>

#include <bcc.h>
#include <bcc/debug.h>
#include <bcc/gencode.h>
#include <bcc/reg.h>
#include <bcc/sc.h>
#include <bcc/scan.h>
#include <bcc/type.h>
#include <bcc/output.h>
#include <bcc/genloads.h>
#include <bcc/label.h>
#include <bcc/codefrag.h>

static char *opname[LASTOP - FIRSTOP + 1] =    /* operator names */
{                /* order must agree with op.h */
    "cond?",
    "or",
    "eor",
    "and",
    "gt", "lt",
    "add",
    "div", "mod",
    "lognot", "not",
    "strucelt", "strucptr",
    "eq",
    "addab", "andab", "divab", "eorab", "modab", "mulab", "orab",
    "slab", "srab", "subab",
    "comma",
    "cond:",
    "logor",
    "logand",
    "logeq",
    "ne",
    "ge", "le",
    "sl", "sr",
    "sub",
    "mul",
    "address", "cast", "indirect", "neg",
    "predec", "preinc", "postdec", "postinc",
    "func", "list", "rootlist",
    "leaf",
    "ptraddab", "ptradd", "ptrsub",
};

static void outindchars(int byte, indn_t count);

void dbitem(struct symstruct *item)
{
    dbtype(item->type);
    if (item->storage == NOSTORAGE)
    {
    outbyte(' ');
    outstr(item->name.namep + 2);
    outstr(" (offset ");
    outshex(item->offset.offi);
    outbyte(')');
    return;
    }
    if (item->storage == LOCAL)
    {
    outbyte(' ');
    if (item->flags == TEMP)
        outstr("(temp)");
    else
        outstr(item->name.namep);
    }
    outstr(" = ");
    outindchars('[', item->indcount);
    switch (item->storage)
    {
    case CONSTANT:
    outstr("const ");
    if (item->type->scalar & RSCALAR)
        outstr("(whatever)");
    else if (item->type->scalar & UNSIGNED)
        outuvalue((uvalue_t) item->offset.offv);
    else
        outvalue(item->offset.offv);
    break;
    case BREG:
    case DREG:
    case INDREG0:
    case INDREG1:
    case INDREG2:
#ifdef DATREG1
    case DATREG1:
#endif
#ifdef DATREG2
    case DATREG2:
#endif
    outregname(item->storage);
    if (item->level == OFFKLUDGELEVEL)
    {
        outplus();
        if (item->flags & LABELLED)
        outlabel(item->name.label);
        else
        outccname(item->name.namep);
    }
    break;
    case LOCAL:
    outbyte('S');
    if (sp <= 0)
        outplus();
    outshex(-sp);
    break;
    case GLOBAL:
    if (item->flags & LABELLED)
        outlabel(item->name.label);
    else
        outstr(item->name.namep);
    break;
    default:
    outstr("bad storage (");
    outhex((uoffset_T) item->storage);
    outbyte(')');
    outstr(" offset ");
    }
    if (item->storage != CONSTANT)
    {
    if (item->offset.offi >= 0)
        outplus();
    outshex(item->offset.offi);
    }
    outindchars(']', item->indcount);
}

void dbtype(struct typestruct *type)
{
    for ( ; type != NULL; type = type->nexttype)
    {
    outbyte(' ');
    switch (type->constructor)
    {
    case ARRAY:
        outbyte('[');
        outhex(type->typesize / type->nexttype->typesize);
        outbyte(']');
        break;
    case FUNCTION:
        outstr("()");
        break;
    case POINTER:
        outbyte('*');
        break;
    case STRUCTU:
        outstr("struct ");
    default:
        if (type->scalar & UNSIGNED)
        outstr("unsigned ");
        outstr(type->tname);
        break;
    }
    }
}

/* sub-nodes must be leaves */
void debug(struct nodestruct *exp)
{
    if (!debugon)
    return;
    comment();
    if (exp->tag < FIRSTOP && exp->tag > LASTOP)
    outstr("unknown op");
    else
    outstr(opname[exp->tag - FIRSTOP]);
    if (exp->right != NULL && exp->tag != FUNCOP &&
    exp->tag != LISTOP && exp->tag != ROOTLISTOP)
    {
    dbitem(exp->right->left.symptr);
    outstr(" to");
    }
    dbitem(exp->left.nodeptr->left.symptr);
    outstr(" (used reg = ");
    if (reguse & INDREG0)
    outregname(INDREG0);
    if (reguse & INDREG1)
    outregname(INDREG1);
    if (reguse & INDREG2)
    outregname(INDREG2);
    outnstr(")");
}

void debugswap()
{
    if (debugon)
    outnstr("* swapping");
}

static void outindchars(int byte, indn_t count)
{
    while (count--)
    outbyte(byte);
}
