/* types.h - type definitions for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#ifndef _BCC_BCC_TYPES_H
#define _BCC_BCC_TYPES_H

#include <stdint.h>

/*
 * source types big enough to handle target quantities
 * these have to be match the source compiler and target machine
 * this is impossible if the source long type is too small
 */

typedef int32_t value_t;            /* target ints, longs and offsets */
typedef uint32_t uvalue_t;  /* target unsigned ints, longs and offsets */

#ifdef I8088
typedef int32_t offset_T;           /* target machine offset */
typedef uint32_t uoffset_T; /* target unsigned machine offset */
#define outuvalue outhex
#define outvalue outshex
#endif
#ifdef MC6809
typedef int offset_T;
typedef unsigned uoffset_T;
#endif


/*
 * storage class type must hold all the flags defined elsewhere
 * it must have a few more bits than the target has registers
 */

#ifdef I8088
typedef uint32_t store_t;    /* storage class flags */
#endif
#ifdef MC6809
#ifdef __STDC__
typedef int store_t;
# else
typedef unsigned store_t;
# endif
typedef unsigned char store_t;
#endif


/*
 * types for library routines
 */

typedef int fd_t;             /* file descriptor */
/*
 * derived scalar types
 * the types containing bit flags fit in an 8 bit smalin_t
 */

typedef uint32_t bool_t;     /* boolean: TRUE if nonzero */
typedef int32_t  ccode_t;    /* condition code code */
typedef int32_t constr_t;   /* type constructor flags */
typedef uint32_t indn_t;     /* storage indirection count */
typedef uint32_t label_no;   /* label number */
typedef uint32_t maclev_t;   /* macro expansion level */
typedef int32_t  op_t;       /* operator code */
typedef uint32_t sc_t;       /* storage class flags */
typedef uint32_t scalar_t;   /* type scalar flags */
typedef uint32_t scopelev_t; /* scope level */
typedef int32_t  sym_t;      /* symbol code from scanner */
typedef uint32_t weight_t;   /* expression tree node weight */


/*
 * derived structure types
 * the fields are ordered in a way that is most space-efficient
 * when smalin_t is char and smalu_t is unsigned char
 * the first element of the structures is the one most frequently accessed
 */

/* expression table entry format */
struct nodestruct
{
    op_t tag;
    weight_t weight;
    uint32_t flags;
    struct typestruct *nodetype;
    union nodeunion
    {
        struct nodestruct *nodeptr;
        struct symstruct *symptr;
    } left;
    struct nodestruct *right;
};

/* symbol table entry format */
struct symstruct
{
    store_t storage;
    indn_t indcount;
    sc_t flags;
    scopelev_t level;
    struct symstruct *next;
    struct symstruct **prev;
    struct typestruct *type;
    union
    {
        float *offd;      /* value for double constants */
        offset_T offi;     /* offset for register or global storage */
        label_no offlabel; /* label number for strings */
        char *offp;        /* to string for macro definitions */
        sym_t offsym;      /* symbol code for keywords */
        value_t offv;      /* value for integral constants */
    } offset;
    union
    {
        label_no label;    /* label number for strings */
        char namea[1];     /* variable length array for declarations */
        char *namep;       /* to constant storage for exprs */
    } name;
};

/* type table entry format */
struct typestruct
{
    scalar_t scalar;             /* scalar type flags u d f l i s c */
    constr_t constructor;        /* constructed type flags a f p s/u */
    char structkey[2];           /* unique prefix for member names
                                  * ranges from "\001\001" to "@\377"
                                  * avoiding nulls */
    uoffset_T alignmask;         /* alignment mask, typesize - 1 for scalars */
    uoffset_T typesize;          /* size of this type */
    char *tname;                 /* name of scalar type or constructor */
    struct typelist *listtype;   /* list of member types */
    struct typestruct *nexttype; /* next in list */
    struct typestruct *prevtype; /* previous in list */
    struct typestruct *sidetype; /* next in sideways list */
};

/* list of type structures */
struct typelist
{
    struct typelist *tlnext;
    struct typestruct *tltype;
};

/* definitions to avoid passing raw NULLs to functions */
#define NULLNODE ((struct nodestruct *) NULL)
#define NULLTYPE ((struct typestruct *) NULL)

#endif /* _BCC_BCC_TYPES_H */
