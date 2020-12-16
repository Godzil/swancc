/* globvar.h - global variables for assembler */

/* global control and bookkeeping */

extern bool_t binaryc;		/* current binary code flag */
extern bool_t binaryg;		/* global binary code flag */
extern offset_t binmbuf;	/* offset in binary code buffer for memory */
extern bool_t binmbuf_set;	/* set to 1 when binmbuf set by org */

extern unsigned char dirpag;	/* direct page */

extern bool_t globals_only_in_obj;	/* global symbols only in object file */

extern bool_t jumps_long;	/* make all jumps long */

extern unsigned char mapnum;	/* global map number */

extern bool_t objectc;		/* current object code flag */
extern bool_t objectg;		/* global object code flag */

extern bool_t pass;		/* pass, FALSE means 0, TRUE means 1 */

extern offset_t progent;	/* program entry point */

extern bool_t symgen;		/* generate symbol table flag */

extern unsigned toterr;		/* total errors */
extern unsigned totwarn;	/* total warnings */

extern bool_t list_force;	/* Force line to be listed - no error */

/* bookeeping for current line */

extern char *linebuf;		/* buffer */

/* for symbol table routines */

extern unsigned char inidata;	/* init sym entry data governed by "u" flag */
extern struct sym_s **spt;	/* symbol pointer table */
extern struct sym_s **spt_top;	/* top of symbol ptr table */

/* for translator */

extern struct sym_s *label;	/* non-null if valid label starts line */
extern unsigned char pedata;	/* shows how PROGENT bound, flags like LCDATA*/
extern unsigned char popflags;	/* pseudo-op flags */

/* for BLOCK stack */

extern struct block_s *blockstak;	/* stack ptr */
extern unsigned char blocklevel;	/* nesting level */

/* for IF stack */

extern struct if_s *ifstak;	/* stack ptr */
extern unsigned char iflevel;	/* nesting level */
extern bool_t ifflag;		/* set if assembling */

/* location counters for various segments */

extern offset_t lc;		/* location counter */
extern unsigned char lcdata;	/* shows how lc is bound */
				/* FORBIT is set if lc is forward referenced */
				/* RELBIT is is if lc is relocat. (not ASEG) */
extern offset_t lcjump; 	/* lc jump between lines */

extern offset_t oldlabel; 	/* Used for checking for moving labels */
#ifdef LOW_BYTE
#define mcount (((unsigned char *) &lcjump)[LOW_BYTE])
				/* low byte of lcjump */
#else
#define mcount lcjump		/* I think this is just a speed hack */
#endif
extern struct lc_s *lcptr;	/* top of current spot in lctab */
extern struct lc_s *lctab;	/* start of lctab */
extern struct lc_s *lctabtop;	/* top of lctab */

/* for code generator */

extern opsize_t mnsize;		/* 1 if forced byte operand size, else 0 */
extern opcode_t page;
extern opcode_t opcode;
extern opcode_t postb;		/* postbyte, 0 if none */
extern unsigned char pcrflag;	/* OBJ_RMASK set if addressing is PC-relative */
extern int last_pass;		/* Pass number of last pass */
extern int dirty_pass;		/* Set if this pass had a label movement */

extern int textseg;		/* Text segment id */

#ifdef I80386

extern opcode_t aprefix;	/* address size prefix or 0 */
extern bool_t asld_compatible;	/* asld compatibility flag */
extern opsize_t defsize;	/* current default size */
extern opsize_t idefsize;	/* initial default size */
extern opcode_t oprefix;	/* operand size prefix or 0 */
extern opcode_t sprefix;	/* segment prefix or 0 */
extern opcode_t sib;		/* scale-index-base byte */

extern int cpuid;		/* Assembler instruction limit flag */
extern int origcpuid;		/* Assembler instruction limit flag */

#endif

/* miscellaneous */

extern char hexdigit[];

/* cpuid functions */
#ifdef I80386
#ifndef __AS386_16__
#define iscpu(x) (cpuid>=(x))
#define needcpu(x) do{ if(cpuid<(x)) {warning(CPUCLASH); cpuid|=0x10;} }while(0)
#define setcpu(x) (cpuid=(x))
#define cpuwarn() (cpuid&=0xF)
#endif
#endif

#ifndef setcpu
#define needcpu(x)
#define setcpu(x)
#define cpuwarn()
#endif

