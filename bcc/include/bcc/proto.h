/* proto.h - extern functions for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#ifndef _BCC_BCC_PROTO_H
#define _BCC_BCC_PROTO_H

/* TODO: This file HAVE to be splited with one for each .c instead of that big mess */

/* assign.c */
void assign(struct symstruct *source, struct symstruct *target);
void cast(struct typestruct *type, struct symstruct *target);
void extend(struct symstruct *target);

/* codefrag.c */
void adjsp(label_no label);
void clrBreg(void);
void comment(void);
void ctoi(void);
void defbyte(void);
void deflong(uoffset_T value);
void defword(void);
void defdword(void);
void even(void);
void negDreg(void);
void comDreg(void);
void outadd(void);
void outaddsp(void);
void outcalladr(void);
void outcmp(void);
void outdec(void);
void outdword(void);
void outfail(void);
void outinc(void);
void outindleft(void);
void outindright(void);
void outindstackreg(void);
void outldaccum(void);
void outldmulreg(void);
void outlea(void);
void outleasp(void);
void outload(void);
void outmulmulreg(void);
void outopsep(void);
void outpshs(void);
void outpuls(void);
void outreturn(void);
void outstore(void);
void outsub(void);
void outtest(void);
void outword(void);
void sctoi(void);
void stoi(void);
void ustoi(void);
void outABX(void);
void outdirectpage(void);
void outextended(void);
void outncspregname(void);
void outindframereg(void);
void adc0(void);
void addconst(offset_T offset, store_pt reg);
void adjlc(offset_T offset, store_pt reg);
void andconst(offset_T offset);
void bssseg(void);
label_no casejump(void);
void common(char *name);
void cseg(void);
void defnulls(uoffset_T nullcount);
label_no defstr(char *sptr, char *stop, bool_pt dataflag);
bool_pt diveasy(value_t divisor, bool_pt uflag);
void dpseg(void);
void dseg(void);
void equ(char *name, char *string);
void equlab(label_no label, offset_T offset);
void globl(char *name);
void import(char *name);
void itol(store_pt reg);
void lcommlab(label_no label);
void lcommon(char *name);
void lea(offset_T offset, store_pt sourcereg, store_pt targreg);
void loadconst(offset_T offset, store_pt reg);
int lslconst(value_t shift, store_pt reg);
int lsrconst(value_t shift, store_pt reg, bool_pt uflag);
bool_pt modeasy(value_t divisor, bool_pt uflag);
bool_pt muleasy(uvalue_t factor, store_pt reg);
void negreg(store_pt reg);
char *opstring(op_pt op);
void outccname(char *name);
void outhiaccum(void);
void outimmadr(offset_T offset);
void outimadj(offset_T offset, store_pt targreg);
void outimmed(void);
void outjumpstring(void);
void outnccname(char *name);
void outncimmadr(offset_T offset);
void outoffset(offset_T offset);
void public (char *name);
void private (char *name);
void regexchange(store_pt sourcereg, store_pt targreg);
void regtransfer(store_pt sourcereg, store_pt targreg);
void sbc0(void);
void set(char *name, offset_T value);
void sl1(store_pt reg);
void slconst(value_t shift, store_pt reg);
void srconst(value_t shift, bool_pt uflag);
void uitol(store_pt reg);
void restoreopreg(void);
void saveopreg(void);

/* debug.c */
void dbitem(struct symstruct *item);
void dbtype(struct typestruct *type);
void debug(struct nodestruct *exp);
void debugswap(void);

/* declare.c */
void colon(void);
void decllist(void);
void lparen(void);
void needvarname(void);
void program(void);
void rbrace(void);
void rbracket(void);
void rparen(void);
void semicolon(void);
struct typestruct *typename (void);

/* express.c */
struct nodestruct *assignment_exp(void);
struct nodestruct *expression(void);

/* exptree.c */
struct nodestruct *castnode(struct typestruct *type, struct nodestruct *nodeptr);
void etreeinit(void);
struct nodestruct *leafnode(struct symstruct *source);
struct nodestruct *node(op_pt t, struct nodestruct *p1, struct nodestruct *p2);

/* floatop.c */
bool_pt f_indirect(struct symstruct *target);
void float1op(op_pt op, struct symstruct *source);
void floatop(op_pt op, struct symstruct *source, struct symstruct *target);
void fpush(struct symstruct *source);
void justpushed(struct symstruct *target);

/* function.c */
void call(char *name);
void function(struct symstruct *source);
void ldregargs(void);
void loadretexpression(void);
void listo(struct symstruct *target, offset_T lastargsp);
void listroot(struct symstruct *target);
void popframe(void);
void reslocals(void);
void ret(void);

/* gencode.c */
void bileaf(struct nodestruct *exp);
fastin_pt bitcount(uvalue_t number);
void codeinit(void);
fastin_pt highbit(uvalue_t number);
void makeleaf(struct nodestruct *exp);

/* genloads.c */
void addoffset(struct symstruct *source);
void address(struct symstruct *source);
void exchange(struct symstruct *source, struct symstruct *target);
store_pt getindexreg(void);
void indexadr(struct symstruct *source, struct symstruct *target);
void indirec(struct symstruct *source);
void load(struct symstruct *source, store_pt targreg);
void loadany(struct symstruct *source);
void loadreg(struct symstruct *source, store_pt targreg);
void makelessindirect(struct symstruct *source);
void movereg(struct symstruct *source, store_pt targreg);
void onstack(struct symstruct *target);
void outadr(struct symstruct *adr);
void outcregname(store_pt reg);
void outncregname(store_pt reg);
void outnregname(store_pt reg);
void outregname(store_pt reg);
void outshortregname(store_pt reg);
void pointat(struct symstruct *target);
void poplist(store_pt reglist);
void push(struct symstruct *source);
void pushlist(store_pt reglist);
void pushreg(store_pt reg);
void storereg(store_pt sourcereg, struct symstruct *target);
void struc(struct symstruct *source, struct symstruct *target);
void transfer(struct symstruct *source, store_pt targreg);

/* glogcode.c */
void cmp(struct symstruct *source, struct symstruct *target, ccode_t *pcondtrue);
void condop(struct nodestruct *exp);
void jumpfalse(struct nodestruct *exp, label_no label);
void jumptrue(struct nodestruct *exp, label_no label);
void logop(struct nodestruct *exp);

/* hardop.c */
void add(struct symstruct *source, struct symstruct *target);
void incdec(op_pt op, struct symstruct *source);
void neg(struct symstruct *target);
void not (struct symstruct *target);
void op1(op_pt op, struct symstruct *source, struct symstruct *target);
void ptrsub(struct symstruct *source, struct symstruct *target);
void sub(struct symstruct *source, struct symstruct *target);

/* input.c */
void closein(void);
void errorloc(void);
void gch1(void);
void include(void);
void openio(int argc, char **argv);
void skipeol(void);
void specialchar(void);
void linecontol(void);

/* label.c */
void bumplc(void);
void bumplc2(void);
void bumplc3(void);
void clearfunclabels(void);
void clearlabels(char *patchbuf, char *patchtop);
void clearswitchlabels(void);
uoffset_T getlc(void);
void deflabel(label_no label);
label_no gethighlabel(void);
label_no getlabel(void);
void jump(label_no label);
void lbranch(ccode_pt cond, label_no label);
struct symstruct *namedlabel(void);
void outcond(ccode_pt cond);
void outlabel(label_no label);
void outnlabel(label_no label);
void sbranch(ccode_pt cond, label_no label);
void unbumplc(void);

/* loadexp.c */
value_t constexpression(void);
void initexpression(struct typestruct *type);
struct typestruct *loadexpression(store_pt targreg, struct typestruct *targtype);

/* longop.c */
void longop(op_pt op, struct symstruct *source, struct symstruct *target);
void long1op(op_pt op, struct symstruct *target);
void outlongendian(void);

/* output.c */
void bugerror(char *message);
void closeout(void);
void error(char *message);
void error2error(char *message1, char *message2);
void fatalerror(char *message);
void finishup(void);
void flushout(void);
void limiterror(char *message);
void initout(void);
void openout(char *oname);
void outbyte(int ch);
void outcomma(void);
void outcpplinenumber(unsigned nr, char *fname, char *str);
void outhex(uoffset_T num);
void outhexdigs(uoffset_T num);
void outline(char *s);
void outminus(void);
void outnl(void);
void outnbyte(int byte);
void outnhex(uoffset_T num);
void outnop1str(char *s);
void outnop2str(char *s);
void outnstr(char *s);
void outop0str(char *s);
void outop1str(char *s);
void outop2str(char *s);
void outop3str(char *s);
void outplus(void);
void outshex(offset_T num);
void outstr(char *s);
void outtab(void);
void outudec(unsigned num);
void outuvalue(uvalue_t num);
void outvalue(value_t num);
char *pushudec(char *s, unsigned num);
void setoutbufs(void);

/* preproc.c */
void blanks(void);
bool_pt blanksident(void);
void checknotinif(void);
void define(void);
void definestring(char *str);
void docontrol(void);
void entermac(void);
void ifinit(void);
int ifcheck(void);
void leavemac(void);
void predefine(void);
char *savedlineptr(void);
void skipcomment(void);
void skipline(void);
void undefinestring(char *str);

/* preserve.c */
void changesp(offset_T newsp, bool_pt absflag);
void loadpres(struct symstruct *source, struct symstruct *target);
void modstk(offset_T newsp);
void pres2(struct symstruct *source, struct symstruct *target);
void preserve(struct symstruct *source);
store_pt preslval(struct symstruct *source, struct symstruct *target);
void recovlist(store_pt reglist);
void savereturn(store_pt savelist, offset_T saveoffset);

/* sc.c */
int main(int argc, char **argv);

/* scan.c */
void cppscan(int asmonly);
void eofin(char *message);
bool_pt isident(void);
void nextsym(void);
void stringorcharconst(void);

/* softop.c */
void softop(op_pt op, struct symstruct *source, struct symstruct *target);

/* state.c */
void compound(void);
void outswoffset(offset_T offset);
void outswstacklab(void);

/* table.c */
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
void growheap(unsigned size);
void *growobject(void *object, unsigned extra);
label_no holdstr(char *sptr, char *stop);
void newlevel(void);
void oldlevel(void);
void ourfree(void *ptr);
void *ourmalloc(unsigned nbytes);
void outofmemoryerror(char *message);
void *qmalloc(unsigned size);
void swapsym(struct symstruct *sym1, struct symstruct *sym2);
void syminit(void);

/* type.c */
struct typestruct *addstruct(char *structname);
struct typestruct *iscalartotype(scalar_pt scalar);
struct typestruct *newtype(void);
void outntypechar(struct typestruct *type);
struct typestruct *pointype(struct typestruct *type);
struct typestruct *prefix(constr_pt constructor, uoffset_T size, struct typestruct *type);
struct typestruct *promote(struct typestruct *type);
struct typestruct *tounsigned(struct typestruct *type);
void typeinit(void);

#endif /* _BCC_BCC_PROTO_H */
