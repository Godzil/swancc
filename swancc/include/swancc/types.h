/* types.h - type definitions for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#ifndef _SWANCC_SWANCC_TYPES_H
#define _SWANCC_SWANCC_TYPES_H

#include <stdint.h>

/*
 * source types big enough to handle target quantities
 * these have to be match the source compiler and target machine
 * this is impossible if the source long type is too small
 */

typedef int32_t value_t;            /* target ints, longs and offsets */
typedef uint32_t uvalue_t;  /* target unsigned ints, longs and offsets */

typedef int32_t offset_T;           /* target machine offset */
typedef uint32_t uoffset_T; /* target unsigned machine offset */
#define outuvalue outhex
#define outvalue outshex

/*
 * storage class type must hold all the flags defined elsewhere
 * it must have a few more bits than the target has registers
 */
typedef uint32_t store_t;    /* storage class flags */

/*
 * types for library routines
 */

typedef int fd_t;             /* file descriptor */


/* scanner codes */
enum scan_states
{
    /* The first group of entries consists of all the values that occur in the
     * switch for cppscan().
     */
    IDENT = 0,
    INTCONST,
#define MAXIDSYM INTCONST    /* IDENT and INTCONST must be the only symofchar[] entries below this */
    FLOATCONST,
#define MAXPPNUMSYM FLOATCONST    /* IDENT, INTCONST and FLOATCONST must be the only symofchar[] entries below this */
    CHARCONST,
    CONTROL,
    SLASH,
    SPECIALCHAR,
    STRINGCONST,

    /* The next group of entries are all the rest of the values that occur in
     *  symofchar[] and so in the switch for nextsym().
     */
    AMPERSAND,         /* ADDRESSOP or ANDOP */
    BADCHAR,
    COLON,             /* also COLONOP */
    COMMA,             /* also COMMAOP */
    DECSYM,            /* PREDECOP or POSTDECOP */
    EOFSYM,
    HYPHEN,            /* NEGOP or SUBOP */
    INCSYM,            /* PREINCOP or POSTINCOP */
    LBRACE,
    LBRACKET,
    LPAREN,
    RBRACE,
    RBRACKET,
    RPAREN,
    SEMICOLON,
    STAR,              /* INDIRECTOP or MULOP */
    WHITESPACE,

    /* The next group of entries consists of all operator codes.  These codes must
     * be contiguous so they can be used as (offsetted) array indexes.  The group
     * is ordered by operator-precedence (this is not necessary).  The first part
     * of it overlaps the previous group.
     */

    /* Assign-abops (level 1) belong here but are at end to improve switch. */
#define FIRSTOP CONDOP
    CONDOP,            /* level 2 */
    OROP,              /* level 5 */
    EOROP,             /* level 6 */
    ANDOP,             /* level 7 */
    GTOP,              /* level 9 */
    LTOP,
    ADDOP,             /* level 11 */
    DIVOP,             /* level 12 */
    MODOP,
    LOGNOTOP,          /* level 13 */
    NOTOP,
    STRUCELTOP,        /* level 14 */
    STRUCPTROP,
    /* End of symbols that appear in symofchar[]. */

    ASSIGNOP,          /* level 1 - assign ops must be contiguous */
    ADDABOP,
    ANDABOP,
    DIVABOP,
    EORABOP,
    MODABOP,
    MULABOP,
    ORABOP,
    SLABOP,
    SRABOP,
    SUBABOP,

    COMMAOP,           /* level 0 */
    COLONOP,           /* level 2 */
    LOGOROP,           /* level 3 */
    LOGANDOP,          /* level 4 */
    EQOP,              /* level 8 */
    NEOP,
    GEOP,              /* level 9 */
    LEOP,
    SLOP,              /* level 10 */
    SROP,
    SUBOP,             /* level 11 */
    MULOP,             /* level 12 */
    ADDRESSOP,         /* level 13 */
    CASTOP,
    INDIRECTOP,
    NEGOP,
    PREDECOP,
    PREINCOP,
    POSTDECOP,
    POSTINCOP,

    FUNCOP,            /* level 14 */
    LISTOP,
    ROOTLISTOP,

    LEAF,              /* special */
    PTRADDABOP,
    PTRADDOP,
    PTRSUBOP,

    /* end of operator codes (they must stay contiguous) */
#define LASTOP PTRSUBOP

    ENUMDECL,
    NULLDECL,
    STRUCTDECL,
    TYPEDECL,
    TYPEDEFNAME,
    UNIONDECL,
    UNSIGNDECL,
    AUTODECL,
    EXTERNDECL,
    REGDECL,
    STATICDECL,
    TYPEDEFDECL,

    ASMSYM,
    BREAKSYM,
    CASESYM,
    CONTSYM,
    DEFAULTSYM,
    DEFINEDSYM,
    DOSYM,
    ELSESYM,
    FORSYM,
    GOTOSYM,
    IFSYM,
    RETURNSYM,
    SIZEOFSYM,
    SWITCHSYM,
    WHILESYM,

    ASMCNTL,
    DEFINECNTL,
    ENDASMCNTL,
    INCLUDECNTL,
    LINECNTL,
    UNDEFCNTL,

    ELIFCNTL,            /* "IF" controls must be contiguous */
    ELSECNTL,
    ENDIFCNTL,
    IFCNTL,
    IFDEFCNTL,
    IFNDEFCNTL
};

/*
 * derived scalar types
 * the types containing bit flags fit in an 8 bit smalin_t
 */
typedef uint32_t bool_t;     /* boolean: TRUE if nonzero */
typedef int32_t  ccode_t;    /* condition code code */
typedef int32_t constr_t;   /* type constructor flags */
typedef uint8_t indn_t;     /* storage indirection count */
typedef uint32_t label_no;   /* label number */
typedef uint32_t maclev_t;   /* macro expansion level */
typedef enum scan_states op_t; /* Operator op */
typedef uint32_t sc_t;       /* storage class flags */
typedef uint32_t scalar_t;   /* type scalar flags */
typedef uint32_t scopelev_t; /* scope level */
typedef enum scan_states sym_t;      /* symbol code from scanner */
typedef uint32_t weight_t;   /* expression tree node weight */


/*
 * derived structure types
 * the fields are ordered in a way that is most space-efficient
 * when smalin_t is char and smalu_t is uint8_t
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
        uint32_t offu;     /* offset for register or global storage */
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

/* Target type sizes */
#define CHAR_TYPE_SIZE (1)
#define DOUBLE_TYPE_SIZE (8)
#define FUNCTION_TYPE_SIZE (0) /* This one is not a type?! */
#define INT_TYPE_SIZE (2)
#define POINTER_TYPE_SIZE (2)
#define SHORT_TYPE_SIZE (2)

#endif /* _SWANCC_SWANCC_TYPES_H */
