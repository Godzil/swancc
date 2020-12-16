/* input.h - input for bcc */

/* Copyright (C) 1992 Bruce Evans */

#define FAKE_INBUFSIZE_1_NOT

#ifdef FAKE_INBUFSIZE_1
#include <stdio.h>
FILE *fdopen();
#endif

struct fcbstruct		/* file control block structure */
{
    fd_t fd;			/* file descriptor */
#ifdef FAKE_INBUFSIZE_1
    FILE *fp;
#endif
    unsigned linenumber;	/* current line in file */
    int lineoffset;		/* offset to start of current line in buf */
    char *lineptr;		/* current spot in line */
    char *limit;		/* end of used part of input buffer */
    struct fbufstruct *includer;
    				/* buffer of file which included current one */
};

extern bool_t asmmode;		/* nonzero when processing assembler code */
				/* depends on zero init */
extern char ch;			/* current char */
extern bool_t cppmode;		/* nonzero if acting as cpp not as compiler */
extern bool_t eofile;		/* nonzero after end of main file reached */
				/* depends on zero init */
extern struct fcbstruct input;	/* current input file control block */
				/* input.lineptr is not kept up to date */
extern char *lineptr;		/* ptr to current char */
extern maclev_t maclevel;	/* nest level of #defined identifiers */
				/* depends on zero init */
extern bool_t orig_cppmode;	/* same as cppmode ex. not varied while in # */
extern bool_t virtual_nl;	/* For -C and asm, don't print first nl */
