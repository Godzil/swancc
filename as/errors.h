/* Error codes. */

/* Syntax errors. */
extern char * COMEXP;           /* "comma expected" */
extern char * DELEXP;           /* "delimiter expected" */
extern char * FACEXP;           /* "factor expected" */
extern char * IREGEXP;          /* "index register expected" */
extern char * LABEXP;           /* "label expected" */
extern char * LPEXP;            /* "left parentheses expected" */
extern char * OPEXP;            /* "opcode expected" */
extern char * RBEXP;            /* "right bracket expected" */
extern char * REGEXP;           /* "register expected" */
extern char * RPEXP;            /* "right parentheses expected" */
extern char * SPEXP;            /* "space expected" */

/* Expression errors. */
extern char * ABSREQ;           /* "absolute expression required" */
extern char * NONIMPREQ;        /* "non-imported expression required" */
extern char * RELBAD;           /* "relocation impossible" */

/* Label errors. */
extern char * ILLAB;            /* "illegal label" */
extern char * MACUID;           /* "MACRO used as identifier" */
extern char * MISLAB;           /* "missing label" */
extern char * MNUID;            /* "opcode used as identifier" */
extern char * REGUID;           /* "register used as identifier" */
extern char * RELAB;            /* "redefined label" */
extern char * UNBLAB;           /* "unbound label" */
extern char * UNLAB;            /* "undefined label" */
extern char * VARLAB;           /* "variable used as label" */

/* Addressing errors. */
extern char * ABOUNDS;          /* "address out of bounds" */
extern char * DBOUNDS;          /* "data out of bounds" */
extern char * ILLMOD;           /* "illegal address mode" */
extern char * ILLREG;           /* "illegal register" */

/* Control structure errors. */
extern char * ELSEBAD;          /* "no matching IF" */
#define ELSEIFBAD       ELSEBAD
extern char * ENDBBAD;          /* "no matching BLOCK" */
#define ENDIFBAD        ELSEBAD
extern char * EOFBLOCK;         /* "end of file in BLOCK" */
extern char * EOFIF;            /* "end of file in IF" */
extern char * EOFLC;            /* "location counter was undefined at end" */
extern char * EOFMAC;           /* "end of file in MACRO" */
extern char * FAILERR;          /* "user-generated error" */

/* Overflow errors. */
extern char * BLOCKOV;          /* "BLOCK stack overflow" */
extern char * BWRAP;            /* "binary file wrap-around" */
extern char * COUNTOV;          /* "counter overflow" */
extern char * COUNTUN;          /* "counter underflow" */
extern char * GETOV;            /* "GET stack overflow" */
extern char * IFOV;             /* "IF stack overflow" */

extern char * LINLONG;          /* "line too long" */
extern char * MACOV;            /* "MACRO stack overflow" */
extern char * OBJSYMOV;         /* "object symbol table overflow" */
extern char * OWRITE;           /* "program overwrite" */
extern char * PAROV;            /* "parameter table overflow" */
extern char * SYMOV;            /* "symbol table overflow" */
extern char * SYMOUTOV;         /* "output symbol table overflow" */

/* I/O errors. */
extern char * OBJOUT;           /* "error writing object file" */

/* Miscellaneous errors. */
extern char * AL_AX_EAX_EXP;    /* "al ax or eax expected" */
extern char * CTLINS;           /* "control character in string" */
extern char * FURTHER;          /* "futher errors suppressed" */
extern char * ILL_IMM_MODE;     /* "illegal immediate mode" */
extern char * ILL_IND_TO_IND;   /* "illegal indirect to indirect" */
extern char * ILL_IND;          /* "illegal indirection" */
extern char * ILL_IND_PTR;      /* "illegal indirection from previous 'ptr'" */
extern char * ILL_SCALE;        /* "illegal scale" */
extern char * ILL_SECTION;      /* "illegal section" */
extern char * ILL_SEG_REG;      /* "illegal segment register" */
extern char * ILL_SOURCE_EA;    /* "illegal source effective address" */
extern char * ILL_SIZE;         /* "illegal size" */
extern char * IMM_REQ;          /* "immediate expression expected" */
extern char * INDEX_REG_EXP;    /* "index register expected" */
extern char * IND_REQ;          /* "indirect expression required" */
extern char * MISMATCHED_SIZE;  /* "mismatched size" */
extern char * NOIMPORT;         /* "no imports with binary file output" */
extern char * REENTER;          /* "multiple ENTER pseudo-ops" */
extern char * REL_REQ;          /* "relative expression required" */
extern char * REPEATED_DISPL;   /* "repeated displacement" */
extern char * SEGREL;           /* "segment or relocatability redefined" */
extern char * SEG_REG_REQ;      /* "segment register required" */
extern char * SIZE_UNK;         /* "size unknown" */
extern char * UNKNOWN_ESCAPE_SEQUENCE; /* "unknown escape sequence" */

extern char * FP_REG_REQ;       /* "FP register required" */
EXTERN char * FP_REG_NOT_ALLOWED; /* "FP register not allowed" */
EXTERN char * ILL_FP_REG;       /* "illegal FP register" */
EXTERN char * ILL_FP_REG_PAIR;  /* "illegal FP register pair" */
EXTERN char * JUNK_AFTER_OPERANDS; /* "junk after operands" */

EXTERN char * ALREADY;          /* "already defined" */
EXTERN char * UNSTABLE_LABEL;   /* "label moved in last pass add -O?" */

/* Warnings. */
EXTERN char * CPUCLASH;         /* "instruction illegal for current cpu" */
EXTERN char * SHORTB;           /* "short branch would do" */
