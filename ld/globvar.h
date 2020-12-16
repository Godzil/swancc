/* globvar.h - global variables for linker */

/* Copyright (C) 1994 Bruce Evans */

#ifndef extern
#define extern
#endif
extern unsigned errcount;		/* count of errors */
extern struct entrylist *entryfirst;	/* first on list of entry symbols */
extern struct modstruct *modfirst;	/* data for 1st module */
extern struct redlist *redfirst;	/* first on list of redefined symbols */

/* K&R _explicitly_ says extern followed by public is OK */
extern char hexdigit[];			/* constant */
extern int  headerless;			/* Don't output header on exe */

extern bin_off_t text_base_value;	/* Base address of text seg */
extern bin_off_t data_base_value;	/* Base or alignment of data seg */
extern bin_off_t heap_top_value;	/* Minimum 'total' value in x86 header */
