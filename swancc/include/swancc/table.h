/* table.h - table handler for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#ifndef _SWANCC_SWANCC_TABLE_H
#define _SWANCC_SWANCC_TABLE_H

extern char *charptr;              /* next free spot in catchall table */
extern char *chartop;              /* spot after last in table */
extern char *char1top;             /* last character spot in table */
extern char *char3top;             /* third last character spot in table */
extern struct symstruct *exprptr;  /* next entry in expression symbol table */
extern struct symstruct *locptr;   /* next entry in local symbol table */
extern struct symstruct locsyms[]; /* local symbol table */

#define TS1
#ifdef TS
uvalue_t ts_n_newtypelist;
uvalue_t ts_s_newtypelist;
uvalue_t ts_n_filename_term;
uvalue_t ts_s_filename_term;
uvalue_t ts_n_filename;
uvalue_t ts_s_filename;
uvalue_t ts_s_filename_tot;
uvalue_t ts_n_pathname;
uvalue_t ts_s_pathname;
uvalue_t ts_s_pathname_tot;
uvalue_t ts_n_inputbuf;
uvalue_t ts_s_inputbuf;
uvalue_t ts_s_inputbuf_tot;
uvalue_t ts_n_includelist;
uvalue_t ts_s_includelist;
uvalue_t ts_s_outputbuf;
uvalue_t ts_n_macstring_ident;
uvalue_t ts_n_macstring_ordinary;
uvalue_t ts_n_macstring_param;
uvalue_t ts_n_macstring_quoted;
uvalue_t ts_n_macstring_term;
uvalue_t ts_s_macstring;
uvalue_t ts_n_defines;
uvalue_t ts_s_defines;
uvalue_t ts_n_macparam;
uvalue_t ts_s_macparam;
uvalue_t ts_s_macparam_tot;
uvalue_t ts_n_macparam_string_ordinary;
uvalue_t ts_n_macparam_string_quoted;
uvalue_t ts_n_macparam_string_term;
uvalue_t ts_s_macparam_string;
uvalue_t ts_s_macparam_string_tot;
uvalue_t ts_s_macparam_string_alloced;
uvalue_t ts_s_macparam_string_alloced_tot;
uvalue_t ts_s_fakeline;
uvalue_t ts_s_fakeline_tot;
uvalue_t ts_n_string;
uvalue_t ts_n_case;
uvalue_t ts_n_case_realloc;
uvalue_t ts_s_case;
uvalue_t ts_s_case_tot;
uvalue_t ts_n_structname;
uvalue_t ts_s_structname;
uvalue_t ts_n_type;
uvalue_t ts_s_type;
uvalue_t ts_n_global;
uvalue_t ts_size_global;
uvalue_t ts_n_holdstr;
uvalue_t ts_size_holdstr;
uvalue_t ts_n_growobj;
uvalue_t ts_size_growobj_wasted;
uvalue_t ts_n_growheap;
uvalue_t ts_s_growheap;
#endif

/* Protypes */
struct symstruct *addglb(char *name, struct typestruct *type);
struct symstruct *addloc(char *name, struct typestruct *type);
struct symstruct *addlorg(char *name, struct typestruct *type);
void addsym(char *name, struct typestruct *type, struct symstruct *symptr);
struct symstruct *constsym(value_t intconst);
void delsym(struct symstruct *symptr);
void dumpglbs(void);
void dumplocs(void);
void dumpstrings(void);
struct symstruct *exprsym(struct symstruct *symptr);
struct symstruct *findlorg(char *name);
struct symstruct *findstruct(char *name);
struct symstruct **gethashptr(char *sname);
void growheap(unsigned long size);
void *growobject(void *object, unsigned long extra);
label_no holdstr(char *sptr, char *stop);
void newlevel(void);
void oldlevel(void);
void ourfree(void *ptr);
void *ourmalloc(uint32_t nbytes);
void outofmemoryerror(char *message);
void *qmalloc(unsigned long size);
void swapsym(struct symstruct *sym1, struct symstruct *sym2);
void syminit(void);

#endif /* _SWANCC_SWANCC_TABLE_H */
