/* codefrag.c - code fragments for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */
#include <stdio.h>

#include <swancc.h>
#include <swancc/codefrag.h>
#include <swancc/byteord.h>
#include <swancc/condcode.h>
#include <swancc/gencode.h>
#include <swancc/label.h>
#include <swancc/output.h>
#include <swancc/reg.h>
#include <swancc/scan.h>
#include <swancc/sizes.h>
#include <swancc/genloads.h>
#include <swancc/state.h>
#include <swancc/table.h>

#define DEFSTR_BYTEMAX 10
#define DEFSTR_DELIMITER '"'
#define DEFSTR_STRINGMAX 40
#define EOS_TEXT '0'
#define MAXPRINTCHAR '~'
#define MINPRINTCHAR ' '

/* segment numbers */

#ifdef I8088
#define CSEG 0
#define outcseg() outop0str(".text\n")
#define DSEG 1
#define outdseg() outop0str(".data\n")
#define BSSSEG 2
#define outbssseg() outop0str(".bss\n")
#endif

#ifdef I8088
static void adjcarry(void);
#endif
static void clr(store_t reg);
static bool_t lowregisDreg(void);
#ifdef I8088
static void outand(void);
static void outequate(void);
# ifdef XENIX_AS
static void outexport(void);
# endif
static void outmovsx(void);
static void outmovzx(void);
static void tfrhilo(void);
static void tfrlohi(void);
#endif

static void outaccum(void);
static void outstackreg(void);
static void opregadr(void);

/* operator and miscellaneous strings */

#ifdef I8088

#define ACCHISTR "ah"
#define ANDSTRING "and\t"
#define DEFSTR_QUOTER '\\'
#define EORSTRING "xor\t"
#define MAX_INLINE_SHIFT 2    /* better 3 for 88, 1 for 186 and above */
#define ORSTRING "or\t"
#define TARGET_FIRST
#define addfactor(reg) (outadd(), outregname(reg), outncregname(DXREG))
#define defstorage() outop0str(".blkb\t")
#define extBnegD() (ctoi(), negDreg())
#define finishfactor()        /* save/add/subfactor() ended already */
#define outadc() outop3str("adc\t")
#define outandac() (outand(), outaccum(), bumplc())
#define outandlo() (outand(), outstr(acclostr))
#define outbimmed() outbyte('*')
#define outcommon() outop0str(".comm\t")
#define outcwd() outnop1str("cwd")
#define outdefstr() outop0str(".ascii\t\"")
#define outexchange() outop1str("xchg\t")
#define outglobl() outop0str(".globl\t")
#ifdef XENIX_AS
#define outimport() outexport()
#else
#define outexport() outop0str("export\t")
#define outimport() outop0str("import\t")
#endif
#ifdef XENIX_AS
#define outj1switch() outop3str("seg\tcs\nbr\t@");
#else
#define outj1switch() outop3str("seg\tcs\nbr\t");
#endif
#define outj2switch() do { outindleft(); outstr(ireg0str); outindright(); bumplc2(); outnl(); } while(0)
#define outlcommon() outop0str("\tlcomm\t")
#define outlswitch() do { outload(); outstr(ireg0str); outncregname(DREG); } while(0)
#define outnc1() outnstr(",*1")
#define outsbc() outop3str("sbb\t")
#define outset() outstr ("\tset\t")
#define outsl() outop2str("shl\t")
#define outsr() outop2str("sar\t")
#define outtransfer() outload()
#define outusr() outop2str("shr\t")
#define outxor() outop2str(EORSTRING)
#define reclaimfactor()    /* factor in DXREG, DXREG now junk */
#define savefactor(reg) regtransfer((reg), DXREG)
#define smiDreg() do { outcwd(); regexchange(DREG, DXREG); } while(0)
#define sr1() do { outsr(); outaccum(); outnc1(); } while(0)
#define subfactor(reg) do { outsub(); outregname(reg); outncregname(DXREG); } while(0)
#define usr1() do {outusr(); outaccum(); outnc1(); } while(0)

static void adjcarry()
{
    outop3str("rcl\t");
    outregname(DXREG);
    outncimmadr((offset_T)9);
    outand();
    bumplc2();
    bumplc2();
    outregname(DXREG);
    outncimmadr((offset_T)0x100);
}

void clrBreg()
{
    outxor();
    outstr(acclostr);
    outncregname(BREG);
}

void comment()
{
    outstr("! ");
}

void ctoi()
{
    outxor();
    outhiaccum();
    outcomma();
    outhiaccum();
    outnl();
}

void defbyte()
{
    outop0str(".byte\t");
}

#ifdef XENIX_AS

void defword()
{
}                /* don't have to print ".word\t" */

#else

void defword()
{
    outop0str(".word\t");
}

#endif

void defdword()
{
    outop0str("dd\t");
}

void even()
{
    outop0str(".even\n");
}

void negDreg()
{
    outop2str("neg\t");
    outnregname(DREG);
}

void comDreg()
{
    outop2str("not\t");
    outnregname(DREG);
}

void outadd()
{
    outop2str("add\t");
}

void outaddsp()
{
    outadd();
    outstackreg();
    outcomma();
    outimmed();
    bumplc2();
}

static void outand()
{
    outop2str(ANDSTRING);
}

#ifdef XENIX_AS
void outcalladr()
{
    outop2str("call\t@");
}

#else

void outcalladr()
{
    outop2str("call\t");
}

#endif

void outcmp()
{
    outop2str("cmp\t");
}

void outdec()
{
    outop1str("dec\t");
}

void outdword()
{
    outstr("dword ");
}

static void outequate()
{
    outop0str("\t=\t");
}

#ifdef XENIX_AS
static void outexport()
{
    outop0str(".globl\t");
}
#endif

void outfail()
{
    outop0str(".fail\t");
}

void outinc()
{
    outop1str("inc\t");
}

#ifdef XENIX_AS
void outindleft()
{
    outbyte('(');
}

void outindright()
{
    outbyte(')');
}

#else

void outindleft()
{
    outbyte('[');
}

void outindright()
{
    outbyte(']');
}

#endif

#ifndef FRAMEPOINTER
void outindstackreg()
{
    outindleft();
    outregname(STACKREG);
    outindright();
}
#endif

void outldaccum()
{
    outload();
    outaccum();
    outcomma();
}

void outldmulreg()
{
    outload();
    outregname(MULREG);
    outcomma();
}

void outlea()
{
    outop2str("lea\t");
}

void outleasp()
{
    outlea();
    outstackreg();
    outcomma();
}

void outload()
{
    outop2str("mov\t");
}

static void outmovsx()
{
    outop3str("movsx\t");
}

static void outmovzx()
{
    outop3str("movzx\t");
}

void outmulmulreg()
{
    outop2str("mul\t");
    outnregname(MULREG);
}

void outopsep()
{
    outcomma();
}

void outpshs()
{
    outop1str("push");
}

void outpuls()
{
    outop1str("pop");
}

void outreturn()
{
    outnop1str("ret");
}

void outstore()
{
    outload();
}

void outsub()
{
    outop2str("sub\t");
}

void outtest()
{
    outop2str("test\t");
}

void outword()
{
    outstr("word ");
}

void sctoi()
{
    outnop1str("cbw");
}

void stoi()
{
    outnop1str("cwde");
}

static void tfrhilo()
{
    outload();
    outstr(acclostr);
    outcomma();
    outhiaccum();
    outnl();
}

static void tfrlohi()
{
    outload();
    outhiaccum();
    outncregname(BREG);
}

void ustoi()
{
    outmovzx();
    outaccum();
    outcomma();
    outshortregname(DREG);
    outnl();
}

#endif /* I8088 */

#ifdef FRAMEREG

void outindframereg()
{
    outindleft();
    outregname(FRAMEREG);
    outindright();
}

#endif

typedef int32_t seg_t;        /* range 0..3 */

static seg_t segment;        /* current seg, depends on init to CSEG = 0 */

/* add carry resulting from char addition */

void adc0()
{
    outadc();
    outhiaccum();
    outncimmadr((offset_T)0);
}

/* add constant to register */

void addconst(offset_T offset, store_t reg)
{
#ifdef I8088
    if ((uoffset_T) offset + 2 <= 4)    /* do -2 to 2  */
    {
        if (reg == ALREG)
        {
            reg = AXREG;
        }    /* shorter and faster */
        do
        {
            if (offset < 0)
            {
                outdec();
                ++offset;
            }
            else        /* if offset == 0, do inc + dec */
            {
                outinc();
                --offset;    /* shouldn't happen and harmless */
            }
            outnregname(reg);
        } while (offset);
    }
    else
#endif
    {
        outadd();
        outimadj(offset, reg);
    }
}

/* adjust lc for signed offset */

void adjlc(offset_T offset, store_t reg)
{
    if (!(reg & CHARREGS))
    {
        bumplc();
        if (!isbyteoffset(offset))
        {
#ifdef I8088
            if ((store_t)reg != AXREG)
#endif
            {
                bumplc();
            }
        }
    }
}

/* adjust stack ptr by adding a labelled constant less current sp */

void adjsp(label_no label)
{
    outaddsp();
    outbyte(LOCALSTARTCHAR);
    outlabel(label);
    if (switchnow != NULL)
    {
        outminus();
        outswstacklab();
    }
    else
    {
        outplus();
        outhex((uoffset_T)-sp);
    }
    outnl();
}

/* and accumulator with constant */

void andconst(offset_T offset)
{
    uint8_t botbits;
    uoffset_T topbits;

    if (((topbits = (offset & ~CHMASKTO & intmaskto)) != 0) &&
         (topbits != (~CHMASKTO & intmaskto)))
        /* if topbits == 0, callers reduce the type */
    {
#ifdef OP1
        outandhi();
        outncimmadr((offset_T) (topbits >> (INT16BITSTO - CHBITSTO)));
#else
        outandac();
        outncimmadr(offset);
        return;
#endif
    }
    if ((botbits = (uint8_t)offset & CHMASKTO) == 0)
    {
        clrBreg();
    }
    else if (botbits != CHMASKTO)
    {
        outandlo();
        outncimmadr((offset_T)botbits);
    }
}

#ifdef I8088

/* set bss segment */

void bssseg()
{
    if (segment != BSSSEG)
    {
        segment = BSSSEG;
        outbssseg();
    }
}

#endif

/* jump to case of switch */

label_no casejump()
{
    label_no jtablelab;

#ifdef I8088
    outlswitch();
    outj1switch();
    outlabel(jtablelab = getlabel());
    outj2switch();
#endif
    return jtablelab;
}

/* clear register to 0 */

static void clr(store_t reg)
{
    loadconst((offset_T)0, reg);
}

/* define common storage */

void common(char *name)
{
#ifdef I8088
    outcommon();
    outccname(name);
    outcomma();
#endif
}

/* set code segment */

void cseg()
{
    if (segment != CSEG)
    {
        segment = CSEG;
        outcseg();
    }
}

/* define long */

void deflong(uoffset_T value)
{
    uoffset_T longhigh;
    uoffset_T longlow;

    longlow = value & (uoffset_T)intmaskto;
    longhigh = (value >> INT16BITSTO) & (uoffset_T)intmaskto;
    defword();
#if DYNAMIC_LONG_ORDER
    if (long_big_endian)
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
    {
        outnhex(longhigh);
    }
#endif
#if DYNAMIC_LONG_ORDER
    else
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
    {
        outnhex(longlow);
        longlow = longhigh;
    }
#endif
    defword();
    outnhex(longlow);
}

/* define null storage */

void defnulls(uoffset_T nullcount)
{
    if (nullcount != 0)
    {
        defstorage();
        outnhex(nullcount);
    }
}

/* define string */

label_no defstr(char *sptr, char *stop, bool_t dataflag)
{
    int byte;            /* promoted char for output */
    label_no strlab;
    seg_t oldsegment;
    int32_t count;        /* range 0..max(DEFSTR_BYTEMAX,DEFSTR_STRMAX) */

#ifdef HOLDSTRINGS
    if (!(bool_t)dataflag)
    {
        return holdstr(sptr, stop);
    }
#endif
    oldsegment = segment;
#ifdef I8088
    dseg();
#endif
    outnlabel(strlab = getlabel());
    byte = (uint8_t)*sptr++;
    while (sptr <= stop)
    {
        if ((uint8_t)byte >= MINPRINTCHAR && (uint8_t)byte <= MAXPRINTCHAR)
        {
            outdefstr();
            count = DEFSTR_STRINGMAX;
            while (count-- > 0 && (uint8_t)byte >= MINPRINTCHAR && (uint8_t)byte <= MAXPRINTCHAR &&
                   sptr <= stop)
            {
#if DEFSTR_DELIMITER - DEFSTR_QUOTER
                if ((uint8_t)byte == DEFSTR_DELIMITER || (uint8_t)byte == DEFSTR_QUOTER)
#else
                    if ((uint8_t) byte == DEFSTR_DELIMITER)
#endif
                {
                    outbyte(DEFSTR_QUOTER);
                }
                outbyte(byte);
                byte = (uint8_t)*sptr++;
            }
            outnbyte(DEFSTR_DELIMITER);
        }
        else
        {
            defbyte();
            count = DEFSTR_BYTEMAX;
            while (count-- > 0 && ((uint8_t)byte < MINPRINTCHAR || (uint8_t)byte > MAXPRINTCHAR) &&
                   sptr <= stop)
            {
                if (count < DEFSTR_BYTEMAX - 1)
                {
                    outcomma();
                }    /* byte separator */
                outhex((uoffset_T)byte);
                byte = (uint8_t)*sptr++;
            }
            outnl();
        }
    }
    defbyte();
    outnbyte(EOS_TEXT);
    switch (oldsegment)
    {
        case CSEG:
            cseg();
            break;
        case DSEG:
            dseg();
            break;
#ifdef I8088
        case BSSSEG:
            bssseg();
            break;
#endif
    }
    return strlab;
}

/* divide D register by a constant if it is easy to do with shifts */

bool_t diveasy(value_t divisor, bool_t uflag)
{
    bool_t sign;

    sign = FALSE;
    if (divisor < 0 && !(bool_t)uflag)
    {
        sign = TRUE;
        divisor = -divisor;
    }
    if (bitcount((uvalue_t)divisor) > 1)
    {
        return FALSE;
    }
    if (divisor == 0)
    {
        clr(DREG);
    }
    else
    {
        if (sign)
        {
            negDreg();
        }
        srconst((value_t)highbit((uvalue_t)divisor), uflag);
    }
    return TRUE;
}

/* set data segment */
void dseg()
{
    if (segment != DSEG)
    {
        segment = DSEG;
        outdseg();
    }
}

/* equate a name to an EOL-terminated string */

void equ(char *name, char *string)
{
    outstr(name);
    outequate();
    outline(string);
}

/* equate a local label to a value */

void equlab(label_no label, offset_T offset)
{
    outbyte(LOCALSTARTCHAR);
    outlabel(label);
    outequate();
    outshex(offset);
    outnl();
}

/* import or export a variable */

void globl(char *name)
{
    outglobl();
    outnccname(name);
}

/* import a variable */

void import(char *name)
{
    outimport();
    outnccname(name);
}

/* extend an int to a long */

void itol(store_t reg)
{
#define TEMP_LABEL_FOR_REGRESSION_TESTS
#ifdef TEMP_LABEL_FOR_REGRESSION_TESTS
    getlabel();
#endif

    if (lowregisDreg())
    {
#ifdef I8088
        outcwd();
        regtransfer(DXREG, reg);
#else
        label_no exitlab;

        clr(reg);
        testhi();
        sbranch(GE, exitlab = getlabel());
        loadconst((offset_T) - 1, reg);
        outnlabel(exitlab);
#endif
    }
    else
    {
        regtransfer(DREG, reg);
        smiDreg();
    }
}

/* define local common storage */

void lcommlab(label_no label)
{
    outlabel(label);
    outlcommon();
}

void lcommon(char *name)
{
    outccname(name);
    outlcommon();
}

/* load constant into given register */
void loadconst(offset_T offset, store_t reg)
{
#ifdef I8088
    if (offset == 0)
    {
        outxor();
        outregname(reg);
        outncregname(reg);
    }
    else
#endif
    {
        outload();
        outregname(reg);
        if (reg != BREG)
        {
            bumplc();
        }
        outncimmadr(offset);
    }
}

/* convert index half of long reg pair into low half of pair */

static bool_t lowregisDreg()
{
#if DYNAMIC_LONG_ORDER
    if (long_big_endian)
#endif
# if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
    {
        return FALSE;
    }
#endif
#if DYNAMIC_LONG_ORDER
    else
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
    {
        return TRUE;
    }
#endif
}

/* partially long shift left register by a constant (negative = infinity) */
int lslconst(value_t shift, store_t reg)
{
    if ((uvalue_t)shift >= INT16BITSTO)
    {
        slconst(shift - INT16BITSTO, lowregisDreg() ? DREG : reg);
        regexchange(reg, DREG);
        clr(lowregisDreg() ? DREG : reg);
        return 0;
    }
#ifdef I8088
    if (shift >= CHBITSTO)
    {
        if (long_big_endian)
        {
            tfrlohi();
            outnop2str("mov\tal,bh");
            outnop2str("mov\tbh,bl");
            outnop2str("sub\tbl,bl");
        }
        else
        {
            outnop2str("mov\tbh,bl");
            outnop2str("mov\tbl,ah");
            tfrlohi();
            clrBreg();
        }
        return (int)shift - CHBITSTO;
    }
#endif
    return (int)shift;
}

/* partially long shift right register by a constant (negative = infinity) */

int lsrconst(value_t shift, store_t reg, bool_t uflag)
{
    if ((uvalue_t)shift >= INT16BITSTO)
    {
        if (lowregisDreg())
        {
            regexchange(reg, DREG);
        }
        srconst(shift - INT16BITSTO, uflag);
        if ((bool_t)uflag)
        {
            uitol(reg);
        }
        else
        {
            itol(reg);
        }
        return 0;
    }
#ifdef I8088
    if (shift >= CHBITSTO)
    {
        if (long_big_endian)
        {
            outnop2str("mov\tbl,bh");
            outnop2str("mov\tbh,al");
            tfrhilo();
            if ((bool_t)uflag)
            {
                ctoi();
            }
            else
            {
                sctoi();
            }
        }
        else
        {
            tfrhilo();
            outnop2str("mov\tah,bl");
            outnop2str("mov\tbl,bh");
            if ((bool_t)uflag)
            {
                outnop2str("sub\tbh,bh");
            }
            else
            {
                regexchange(reg, DREG);
                sctoi();
                regexchange(reg, DREG);
            }
        }
        return (int)shift - CHBITSTO;
    }
#endif
    return (int)shift;
}

/* take D register modulo a constant if it is easy to do with a mask */

bool_t modeasy(value_t divisor, bool_t uflag)
{
    bool_t sign;

    sign = FALSE;
    if (divisor < 0 && !(bool_t)uflag)
    {
        sign = TRUE;
        divisor = -divisor;
    }
    if (bitcount((uvalue_t)divisor) > 1)
    {
        return FALSE;
    }
    if (--divisor == 0)
    {
        clrBreg();        /* original divisor 1 or -1 yields 0 */
    }
    else
    {
        if (sign)
        {
            negDreg();
        }
        andconst((offset_T)divisor);    /* if original divisor 0, this is
                       null */
        if (sign)
        {
            negDreg();
        }
    }
    return TRUE;
}

/* multiply register by a constant if it is easy to do with shifts */
bool_t muleasy(uvalue_t factor, store_t reg)
{
    int mulstack[MAXINTBITSTO / 2 + 1];    /* must be signed, not a fastin_t */
    int32_t count;
    int32_t single1skip;
    int32_t lastcount;
    int32_t mulsp;
    int stackentry;        /* signed */

#ifdef I8088
    /* Now using imul directly so don't be so keen to shift */
    if (factor > 16 && factor != 32 && factor != 64 && factor != 0xFFFFFFFFL)
    {
        return FALSE;
    }
#endif

    if (factor == 0)
    {
        clr(reg);
        return TRUE;
    }
    single1skip = 0;
    mulsp = -1;            /* may be unsigned, but bumps to 0 */
    while (factor != 0)
    {
        for (lastcount = single1skip ; (factor & 1) == 0 ; factor >>= 1)
        {
            ++lastcount;
        }
        mulstack[(int)++mulsp] = lastcount;
        /* first time bumps mulsp to 0 even if an uint8_t */
        for (count = 0 ; (factor & 1) != 0 ; factor >>= 1)
        {
            ++count;
        }
        single1skip = 1;
        if (count == 2 && factor == 0)
        {
            /* 3 = 2 + 1  better than  3 = 4 - 1 */
            /* but rest of algorithm messed up unless factor now 0 */
            mulstack[(int)++mulsp] = 1;
        }
        else if (count > 1)
        {
            single1skip = 0;
            if (lastcount == 1 && mulsp != 0)
            {
                mulstack[(int)mulsp] = -1 - count;
            }
            else
            {
                mulstack[(int)++mulsp] = -count;
            }
        }
    }
    if (mulsp > 3)
    {
        return FALSE;
    }
    if (mulsp != 0)
    {
        savefactor(reg);    /* on stack or in reg as nec */
        do
        {
            finishfactor();    /* finish save/add/subfactor() if nec */
            stackentry = mulstack[(int)mulsp--];
            if (stackentry < 0)
            {
#ifdef I8088
                if (stackentry == -INT32BITSTO)
                {
                    clr(reg);    /* shifting would do nothing */
                }
                else
#endif
                {
                    slconst((value_t)-stackentry, reg);
                }
                subfactor(reg);    /* from wherever put by savefactor() */
            }
            else
            {
                slconst((value_t)stackentry, reg);
                addfactor(reg);    /* from wherever put by savefactor() */
            }
        } while (mulsp != 0);
        reclaimfactor();    /* reclaim storage if nec */
    }
    slconst((value_t)mulstack[0], reg);
    return TRUE;
}

/* negate a register */

void negreg(store_t reg)
{
    if ((store_t)reg == BREG)
        extBnegD();
    else
    {
        negDreg();
    }
}

/* return string of operator */

char *opstring(op_t op)
{
    switch (op)
    {
        case ANDOP:
            return ANDSTRING;
        case EOROP:
            return EORSTRING;
        case OROP:
            return ORSTRING;
    }
    return "badop";
}

/* print DREG (accumulator) */

static void outaccum()
{
    outstr(accumstr);
}

/* print a c compiler name with leading CCNAMEPREXFIX */

void outccname(char *name)
{
    outbyte(CCNAMEPREFIX);
    outstr(name);
}

/* print high byte of word accumulator */

void outhiaccum()
{
    outstr(ACCHISTR);
}

/* print immediate address */

void outimmadr(offset_T offset)
{
#ifdef I8088
    if (!isbyteoffset(offset))
    {
        outimmed();
    }
    else
        outbimmed();
#else
    outimmed();
#endif
    outshex(offset);
}

/* print register, comma, immediate address and adjust lc */

void outimadj(offset_T offset, store_t targreg)
{
    outregname(targreg);
    adjlc(offset, targreg);
    outncimmadr(offset);
}

/* print immediate address designator */

void outimmed()
{
    outbyte('#');
}

void outjumpstring()
{
    outop3str(jumpstring);
}

/* print cc name, then newline */

void outnccname(char *name)
{
    outccname(name);
    outnl();
}

/* print separator, immediate address, newline */

void outncimmadr(offset_T offset)
{
#ifdef I8088
    outcomma();
#endif
    outimmadr(offset);
    outnl();
}

/* print signed offset and adjust lc */
void outoffset(offset_T offset)
{
    adjlc(offset, INDREG0);
    outshex(offset);
}

/* print stack register */
static void outstackreg()
{
    outstr(stackregstr);
}

void public(char *name)
{
#ifndef AS09
    outexport();
    outnccname(name);
#endif
    outccname(name);
    outnbyte(PUBLICENDCHAR);
}

/* print cc name as a private label */
void private(char *name)
{
#ifdef LABELENDCHAR
    outccname(name);
    outnbyte(LABELENDCHAR);
#else
    outnccname(name);
#endif
}

/* exchange registers */
void regexchange(store_t sourcereg, store_t targreg)
{
    outexchange();
    outregname(sourcereg);
    outncregname(targreg);
#ifdef I8088
    if (!((sourcereg | targreg) & AXREG))
    {
        bumplc();
    }
#endif
}

/* transfer a register */
void regtransfer(store_t sourcereg, store_t targreg)
{
    outtransfer();
#ifdef TARGET_FIRST
    outregname(targreg);
    outncregname(sourcereg);
#else
    outregname(sourcereg);
    outncregname(targreg);
#endif
}

/* subtract carry resulting from char addition */
void sbc0()
{
    outsbc();
    outhiaccum();
    outncimmadr((offset_T)0);
}

/* set a name to a value */
void set(char *name, offset_T value)
{
    outccname(funcname);
    outbyte(LOCALSTARTCHAR);
    outstr(name);
    outset();
    outshex(value);
    outnl();
}

/* shift left register by 1 */
void sl1(store_t reg)
{
    outsl();
#ifdef I8088
    outregname(reg);
    outnc1();
#endif
}

/* shift left register by a constant (negative = infinity) */
void slconst(value_t shift, store_t reg)
{
    if ((uvalue_t)shift >= INT16BITSTO)
    {
        clr(reg);
    }
    else
    {
        if (shift >= CHBITSTO && reg == DREG)
        {
            tfrlohi();
            clrBreg();
            shift -= CHBITSTO;
        }
#ifdef I8088
# if MAX_INLINE_SHIFT < INT16BITSTO
        if (shift > MAX_INLINE_SHIFT)
        {
            outload();
            outregname(SHIFTREG);
            outcomma();
            outimmadr((offset_T)shift);
            outnl();
            outsl();
            outregname(reg);
            outncregname(SHIFTREG);
        }
        else
# endif
#endif
        {
            while (shift--)
            {
                sl1(reg);
            }
        }
    }
}

/* shift right D register by a constant (negative = infinity) */
void srconst(value_t shift, bool_t uflag)
{
    if ((uvalue_t)shift >= INT16BITSTO)    /* covers negatives too */
    {
        if ((bool_t)uflag)
        {
            clr(DREG);
        }
        else            /* make D == 0 if D >= 0, else D == -1 */
            smiDreg();        /* special case of 68020 Scc instruction */
    }
    else
    {
        if (shift >= CHBITSTO)
        {
            tfrhilo();
            if ((bool_t)uflag)
            {
                ctoi();
            }
            else
            {
                sctoi();
            }
            shift -= CHBITSTO;
        }
#ifdef I8088
# if MAX_INLINE_SHIFT < INT16BITSTO
        if (shift > MAX_INLINE_SHIFT)
        {
            outload();
            outregname(SHIFTREG);
            outcomma();
            outimmadr((offset_T)shift);
            outnl();
            if ((bool_t)uflag)
                outusr();
            else
                outsr();
            outaccum();
            outncregname(SHIFTREG);
        }
        else
# endif
#endif
        {
            while (shift--)
            {
                if ((bool_t)uflag)
                    usr1();
                else
                    sr1();
            }
        }
    }
}

/* extend an unsigned in DREG to a long */
void uitol(store_t reg)
{
    if (lowregisDreg())
    {
        clr(reg);
    }
    else
    {
        regexchange(DREG, reg);
        clr(DREG);
    }
}

static char opregstr[] = "_opreg";

/*
 * opregadr()
 * outputs address of variable opreg where OPREG is saved
 */

static void opregadr()
{
#ifdef I8088
    outindleft();
    outccname(opregstr);
    outindright();
    bumplc2();
#endif
}

/*
 * restoreopreg()
 * restores register OPREG from static location >opreg if it is was use
 */
void restoreopreg()
{
    if (reguse & OPREG)
    {
#ifdef I8088
        outload();
        outregname(OPREG);
        outopsep();
        opregadr();
        outnl();
#endif
    }
}

/*
 * saveopreg()
 * saves register OPREG to static location >opreg if it is in use
 * this makes the flop routines non-reentrant. It is too messy to
 * push it because the flop routines leave results on the stack
 */
void saveopreg()
{
    if (reguse & OPREG)
    {
#ifdef I8088
        bssseg();
        common(opregstr);
        outnhex(opregsize);
        cseg();
        outstore();
        opregadr();
        outncregname(OPREG);
#endif
    }
}
