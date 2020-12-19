/* label.c - label handling routines for swancc
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
#include <swancc/condcode.h>
#include <swancc/gencode.h>
#include <swancc/label.h>
#include <swancc/output.h>
#include <swancc/sc.h>
#include <swancc/scan.h>
#include <swancc/sizes.h>
#include <swancc/type.h>
#include <swancc/codefrag.h>
#include <swancc/table.h>

#define outlbranch() outop3str( "b")
#define outsbranch() outop2str( "j")

#define MAXVISLAB 32

struct labdatstruct
{
    label_no labnum;        /* 0 if not active */
    offset_T lablc;        /* location counter for branch or label */
    char *labpatch;        /* buffer ptr for branch, NULL for label */
    ccode_t labcond;        /* condition code for branch */
};

static char lcondnames[][2] =    /* names of long condition codes */
{
    {'e', 'q',}, {'n', 'e',}, {'r', ' ',}, {'r', 'n',},
    {'l', 't',}, {'g', 'e',}, {'l', 'e',}, {'g', 't',},
    {'l', 'o',}, {'h', 'i',}, {'l', 'o',}, {'h', 'i'},
};
static char scondnames[][2] =    /* names of short condition codes */
{
    {'e', ' ',}, {'n', 'e',}, {'m', 'p',}, {'n', 0,},
    {'l', ' ',}, {'g', 'e',}, {'l', 'e',}, {'g', ' ',},
    {'b', ' ',}, {'a', 'e',}, {'b', 'e',}, {'a', ' ',},
};

static label_no lasthighlab = 0xFFFF + 1;    /* temp & temp init so labels fixed */
/* lint */
static label_no lastlab;    /* bss init to 0 */
static offset_T lc;        /* bss init to 0 */

static struct labdatstruct vislab[MAXVISLAB];    /* bss, all labnum's init 0 */
static int32_t nextvislab;    /* bss init to NULL */
static struct symstruct *namedfirst;    /* bss init to NULL */
static struct symstruct *namedlast;    /* bss init to NULL */

static void addlabel(ccode_t cond, label_no label, char *patch);
static struct labdatstruct *findlabel(label_no label);

/* add label to circular list */
static void addlabel(ccode_t cond, label_no label, char *patch)
{
    struct labdatstruct *labptr;

    labptr = &vislab[(int)nextvislab];
    labptr->labcond = cond;
    labptr->labnum = label;
    labptr->lablc = lc;
    labptr->labpatch = patch;
    if (++nextvislab == MAXVISLAB)
    {
        nextvislab = 0;
    }
}

/* bump location counter */
void bumplc()
{
    ++lc;
}

/* bump location counter by 2 */
void bumplc2()
{
    lc += 2;
}

/* bump location counter by 3 */
void bumplc3()
{
    lc += 3;
}

/* clear out labels in function */
void clearfunclabels()
{
    struct symstruct *symptr;
    struct symstruct *tmp;

    for (symptr = namedfirst ; symptr != NULL ;)
    {
        if (symptr->indcount == 2)
        {
            error("undefined label");
        }
        symptr->indcount = 0;
        tmp = symptr;
        symptr = (struct symstruct *)symptr->type;
        tmp->type = NULL;
    }
    namedlast = namedfirst = NULL;
}

/* clear out labels no longer in buffer */

void clearlabels(char *patchbuf, char *patchtop)
{
    struct labdatstruct *labptr;
    struct labdatstruct *labtop;
    char *labpatch;

    for (labptr = &vislab[0], labtop = &vislab[MAXVISLAB] ; labptr < labtop ; ++labptr)
    {
        if ((labpatch = labptr->labpatch) >= patchbuf && labpatch < patchtop)
        {
            labptr->labnum = 0;
        }
    }
}

/* clear out labels in switch statement */
void clearswitchlabels()
{
    struct symstruct *symptr;

    for (symptr = namedfirst ; symptr != NULL ; symptr = (struct symstruct *)symptr->type)
    {
        if (symptr->indcount == 3)
        {
            equlab(symptr->offset.offlabel, lowsp);
            symptr->indcount = 4;
        }
    }
}

/* return location counter */
uoffset_T getlc()
{
    return (uoffset_T)lc;
}

/* define location of label and backpatch references to it */

void deflabel(label_no label)
{
    char *cnameptr;
    struct labdatstruct *labmin;
    struct labdatstruct *labmax;
    struct labdatstruct *labmid;
    struct labdatstruct *labptrsave;
    offset_T nlonger;

    outnlabel(label);
    {
        struct labdatstruct *labptr;
        char *labpatch;

        labmin = &vislab[0];
        labmax = &vislab[MAXVISLAB];
        labptr = labmid = &vislab[(int)nextvislab];
        if (!watchlc)
        {
            do
            {
                if (labptr == labmin)
                {
                    labptr = &vislab[MAXVISLAB];
                }
                --labptr;
                if (labptr->labnum == label)
                {
                    if ((labpatch = labptr->labpatch) != NULL && isshortbranch(lc - labptr->lablc))
                    {
                        /* patch "swancc(c) to j(c)(c)( ) */
                        *labpatch = 'j';
                        *(labpatch + 1) = *(cnameptr = scondnames[(int)labptr->labcond]);

                        *(labpatch + 2) = *(cnameptr + 1);
                        *(labpatch + 3) = ' ';
                        nlonger = jcclonger;
                        if (labptr->labcond == RA)
                        {
                            nlonger = jmplonger;
                        }
                        lc -= nlonger;
                        labptrsave = labptr;
                        while (++labptr != labmid)
                        {
                            if (labptr == labmax)
                            {
                                /* WTF: labptr = &vislab[-1]; */
                                labptr = &vislab[0];
                            }
                            else
                            {
                                labptr->lablc -= nlonger;
                            }
                        }
                        labptr = labptrsave;
                    }
                }
            } while (labptr != labmid);
        }
    }
    addlabel((ccode_t)0, label, (char *)NULL);
}

static struct labdatstruct *findlabel(label_no label)
{
    struct labdatstruct *labptr;
    struct labdatstruct *labtop;

    for (labptr = &vislab[0], labtop = &vislab[MAXVISLAB] ; labptr < labtop ; ++labptr)
    {
        if (labptr->labnum == label)
        {
            if (labptr->labpatch != 0)
            {
                break;
            }
            return labptr;
        }
    }
    return (struct labdatstruct *)NULL;
}

/* reserve a new label, from top down to temp avoid renumbering low labels */
label_no gethighlabel()
{
    return --lasthighlab;
}

/* reserve a new label */
label_no getlabel()
{
    return ++lastlab;
}

/* jump to label */
void jump(label_no label)
{
    lbranch(RA, label);
}

/* long branch on condition to label */
void lbranch(ccode_t cond, label_no label)
{
    char *cnameptr;
    struct labdatstruct *labptr;
    char *oldoutptr;

    if ((ccode_t)cond == RN)
    {
        return;
    }
    if ((labptr = findlabel(label)) != NULL && isshortbranch(lc - labptr->lablc + 2))
    {
        sbranch(cond, label);
        return;
    }
    oldoutptr = outbufptr;
    if (cond == RA)
    {
        outjumpstring();
    }
    else
    {
        outlbranch();
        outbyte(*(cnameptr = lcondnames[(int)cond]));
        outbyte(*(cnameptr + 1));
        if ((ccode_t)cond == LS || (ccode_t)cond == HS)
        {
            outbyte('s');    /* "blos" or "bhis" */
        }
        else
        {
            outbyte(' ');
        }
        outtab();
        bumplc2();
    }
    outlabel(label);
    outnl();
    if (labptr == NULL && oldoutptr < outbufptr)
    {    /* no wrap-around */
        addlabel(cond, label, oldoutptr);
    }
}

/* look up the name gsname in label space, install it if new */
struct symstruct *namedlabel()
{
    struct symstruct *symptr;

    gs2name[1] = 0xFF;
    if ((symptr = findlorg(gs2name + 1)) == NULL)
    {
        symptr = addglb(gs2name + 1, vtype);
        symptr->flags = LABELLED;
    }
    if (symptr->indcount < 2)
    {
        symptr->indcount = 2;
        symptr->offset.offlabel = gethighlabel();
        if (namedfirst == NULL)
        {
            namedfirst = symptr;
        }
        else
        {
            namedlast->type = (struct typestruct *)symptr;
        }
        namedlast = symptr;
        symptr->type = NULL;
    }
    return symptr;
}

/* print label */
void outlabel(label_no label)
{
    outbyte(LABELSTARTCHAR);
    outhexdigs((uoffset_T)label);
}

/* print label and newline */
void outnlabel(label_no label)
{
    outlabel(label);
#ifdef LABELENDCHAR
    outnbyte(LABELENDCHAR);
#else
    outnl();
#endif
}

/* short branch on condition to label */

void sbranch(ccode_t cond, label_no label)
{
    char *cnameptr;

    if ((ccode_t)cond != RN)
    {
        outsbranch();
        outbyte(*(cnameptr = scondnames[(int)cond]));
        outbyte(*(cnameptr + 1));
        outtab();
        outlabel(label);
        outnl();
    }
}

/* reverse bump location counter */
void unbumplc()
{
    --lc;
}