/* vodefrag.h - swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#ifndef _BCC_BCC_CODEFRAG_H
#define _BCC_BCC_CODEFRAG_H

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
void addconst(offset_T offset, store_t reg);
void adjlc(offset_T offset, store_t reg);
void andconst(offset_T offset);
void bssseg(void);
label_no casejump(void);
void common(char *name);
void cseg(void);
void defnulls(uoffset_T nullcount);
label_no defstr(char *sptr, char *stop, bool_t dataflag);
bool_t diveasy(value_t divisor, bool_t uflag);
void dpseg(void);
void dseg(void);
void equ(char *name, char *string);
void equlab(label_no label, offset_T offset);
void globl(char *name);
void import(char *name);
void itol(store_t reg);
void lcommlab(label_no label);
void lcommon(char *name);
void lea(offset_T offset, store_t sourcereg, store_t targreg);
void loadconst(offset_T offset, store_t reg);
int lslconst(value_t shift, store_t reg);
int lsrconst(value_t shift, store_t reg, bool_t uflag);
bool_t modeasy(value_t divisor, bool_t uflag);
bool_t muleasy(uvalue_t factor, store_t reg);
void negreg(store_t reg);
char *opstring(op_t op);
void outccname(char *name);
void outhiaccum(void);
void outimmadr(offset_T offset);
void outimadj(offset_T offset, store_t targreg);
void outimmed(void);
void outjumpstring(void);
void outnccname(char *name);
void outncimmadr(offset_T offset);
void outoffset(offset_T offset);
void public (char *name);
void private (char *name);
void regexchange(store_t sourcereg, store_t targreg);
void regtransfer(store_t sourcereg, store_t targreg);
void sbc0(void);
void set(char *name, offset_T value);
void sl1(store_t reg);
void slconst(value_t shift, store_t reg);
void srconst(value_t shift, bool_t uflag);
void uitol(store_t reg);
void restoreopreg(void);
void saveopreg(void);

#endif /* _BCC_BCC_CODEFRAG_H */
