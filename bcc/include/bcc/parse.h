/* parse.h - parser for bcc */

/* Copyright (C) 1992 Bruce Evans */

/* possible scope levels */

#define ARGLEVEL	1
#define GLBLEVEL	0
#define MAXLEVEL	125
#define MINLOCLEVEL	1

/* possible node flags */

#define LVALUE		(1 << 0)

extern struct nodestruct *etptr;     /* ptr to next entry in expression tree */
extern struct symstruct *gvarsymptr; /* gsymptr for last identifier declared */
extern scopelev_t level;	/* scope level */
				/* depends on zero init */
