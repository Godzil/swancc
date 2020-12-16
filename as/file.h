/* file.h - global variables involving files for assembler */

extern char *filnamptr;		/* file name pointer */
extern char *truefilename;	/* in case actual source name is a tmpname */

extern fd_t infil;		/* current input file (stacked, 0 = memory) */

/* Output fds */
extern unsigned char outfd;	/* output fd for writer fns */
extern fd_t binfil;		/* binary output file (0 = memory) */
extern fd_t lstfil;		/* list output file (0 = standard) */
extern fd_t objfil;		/* object output file */
extern fd_t symfil;		/* symbol table output file */

/* readsrc internals */
extern unsigned infil0;		/* Number of first input area */
extern unsigned infiln;		/* Number of current input area */
