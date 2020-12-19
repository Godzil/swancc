/* gencode.h - code generation for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#ifndef _SWANCC_SWANCC_GENCODE_H
#define _SWANCC_SWANCC_GENCODE_H

#define EXPRLEVEL 126              /* level for symbols in exptree, > real levs */
#define OFFKLUDGELEVEL 127         /* level for expr sym with offset from name */
#define OPERANDSEPARATOR ','       /* char separating operands */
#define OPSEPARATOR '\t'           /* char separating op string and operand */

extern uoffset_T arg1size;         /* size of 1st arg to function zero after allocation of 1st arg */
extern store_t callee1mask;       /* calleemask with doubleregs masked if nec */
extern uoffset_T dataoffset;       /* amount of initialized data so far */
#ifdef DEBUG
extern bool_t debugon;             /* nonzero to print debugging messages depends on zero init */
#endif
#ifdef FRAMEPOINTER
extern store_t framelist;         /* bit pattern for frame and saved regs */
extern store_t frame1list;        /* framelist with doubleregs masked if nec */
extern offset_T framep;            /* hardware relative frame ptr */
#endif
extern uoffset_T func1saveregsize; /* choice of next two values */
extern uoffset_T funcdsaveregsize; /* funcsaveregsize adjusted for doubles */
extern uoffset_T funcsaveregsize;  /* tot size of framelist/calleemask regs */
#ifdef I80386
extern bool_t i386_32;             /* nonzero to generate 386 32 bit code depends on zero init */
#endif
#ifdef DYNAMIC_LONG_ORDER
extern bool_t long_big_endian;    /* nonzero if high long word is first */
                /* depends on zero init */
#endif
extern offset_T lowsp;             /* low water sp (collects locals in switch) */
#ifdef POSINDEPENDENT
extern bool_t posindependent;    /* nonzero to generate pos-independent code */
                /* depends on zero init */
#endif
extern bool_t printf_fp;           /* nonzero if *printf called with FP arg  */
extern bool_t regarg;              /* nonzero to show unloaded register arg depends on zero init */
extern store_t reguse;             /* registers in use */
extern bool_t scanf_fp;            /* nonzero if *scanf called with ptr-to-FP */
extern offset_T softsp;            /* software sp (leads sp during declares) */
extern offset_T sp;                /* hardware relative stack ptr depends on zero init */
#ifdef FRAMEPOINTER
extern bool_t stackarg;            /* nonzero to show function has arg on stack */
#endif
extern struct switchstruct *switchnow; /* currently active switch depends on NULL init */
extern bool_t optimise;            /* nonzero to add optimisation code */

/* variables to be initialised to nonzero */
extern store_t allindregs;        /* mask (in) for index registers */
extern store_t allregs;           /* mask (in) for registers */
extern bool_t arg1inreg;           /* nonzero to pass 1st arg in reg */
extern store_t calleemask;        /* mask (in) for regs to be saved by callee */
extern bool_t callersaves;         /* nonzero to make caller save regs */
extern char *callstring;           /* opcode string for call */
extern store_t doubleargregs;     /* mask (in) for regs for 1st arg if double */
extern store_t doubleregs;        /* mask (in) for regs to temp contain double */
extern store_t doublreturnregs;   /* mask (in) for regs for returning double */
extern offset_T jcclonger;         /* amount jcc long jumps are longer */
extern offset_T jmplonger;         /* amount long jumps is longer */
extern char *jumpstring;           /* opcode string for jump */
extern char *regpulllist;          /* reg names and sizes (0 store_t bit first) */
extern char *regpushlist;          /* reg names and sizes (0 store_t bit last) */
extern store_t regregs;           /* mask (in) for regs which can be reg vars */

/* register names */
extern char *acclostr;
extern char *accumstr;
extern char *badregstr;
#ifdef I8088
extern char *dreg1str;
extern char *dreg1bstr;
extern char *dreg2str;
#endif
extern char *ireg0str;
extern char *ireg1str;
extern char *ireg2str;
extern char *localregstr;
#ifdef I8088
extern char *stackregstr;
#endif

/* register sizes */
extern uoffset_T accregsize;
#ifdef FRAMEPOINTER
extern uoffset_T frameregsize;
#endif
extern uoffset_T maxregsize;
extern uoffset_T opregsize;
extern uoffset_T pshregsize;
extern uoffset_T returnadrsize;

/* Prototypes */

void bileaf(struct nodestruct *exp);
int32_t bitcount(uvalue_t number);
void codeinit(void);
int32_t highbit(uvalue_t number);
void makeleaf(struct nodestruct *exp);

#endif /* _SWANCC_SWANCC_GENCODE_H */
