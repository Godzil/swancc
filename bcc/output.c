/* output.c - output and error handling for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <bcc.h>
#include <bcc/input.h>
#include <bcc/os.h>
#include <bcc/sizes.h>
#include <bcc/table.h>
#include <bcc/output.h>
#include <bcc/codefrag.h>
#include <bcc/label.h>

/* Global variables */
bool_t ctext;        /* nonzero to intermix C source depends on zero init */
char *outbufptr;     /* current spot in output buffer */
char *outbuftop;     /* top of current output buffer */
bool_t watchlc;      /* nonzero to print lc after every line depends on zero init */

#ifdef XENIX_AS
# define HEXSTARTCHAR '/'
#else
# define HEXSTARTCHAR '$'
#endif
#define OUTBUFSIZE 2048
#define opcodeleadin()        /* outtab() for fussy assemblers */

static unsigned errcount;    /* # errors in compilation */
/* depends on zero init */
static char hexdigits[] = "0123456789ABCDEF";
static char *outbuf;
char *outbufend;        /* end of pair of output buffers */
static char *outbufmid;
static fd_t output;
static int32_t outstage;    /* depends on zero init */

static void errorsummary(void);

static void errsum1(void);

#ifdef MC6809
#ifdef DEBUG
static void outvaldigs(uvalue_t num);
#endif
#endif

void bugerror(char *message)
{
    error2error("compiler bug - ", message);
}

void closeout()
{
    char *saveoutptr;

    if (outstage == 3)
    {
        saveoutptr = outbufptr;
        flushout();        /* buffer from last stage */
        outbufptr = saveoutptr;
    }
    outstage = 0;        /* flush from top to current ptr */
    flushout();
    close(output);
}

/* error handler */
void error(char *message)
{
    error2error(message, "");
}

/* error handler - concatenate 2 messages */
void error2error(char *message1, char *message2)
{
    char *warning;

    if (message1[0] == '%' && message1[1] == 'w')
    {
        message1 += 2;
        warning = "warning: ";
    }
    else
    {
        ++errcount;
        warning = "error: ";
    }
    if (output != 1)
    {
        char *old_outbuf;
        char *old_outbufptr;
        char *old_outbuftop;
        fd_t old_output;
        int32_t old_outstage;
        char smallbuf[81];    /* don't use heap - might be full or busy */

        old_outbuf = outbuf;
        old_outbufptr = outbufptr;
        old_outbuftop = outbuftop;
        old_output = output;
        old_outstage = outstage;

        outbufptr = outbuf = &smallbuf[0];
        outbuftop = &smallbuf[sizeof smallbuf];
        output = 1;
        outstage = 0;
        errorloc();
        outstr(warning);
        outstr(message1);
        outstr(message2);
        outnl();
        flushout();

        outbuf = old_outbuf;
        outbufptr = old_outbufptr;
        outbuftop = old_outbuftop;
        output = old_output;
        outstage = old_outstage;
    }
    comment();
    errorloc();
    outstr(warning);
    outstr(message1);
    outstr(message2);
    outnl();
}

/* summarise errors */
static void errorsummary()
{
    if (errcount != 0)
    {
        outfail();
        errsum1();
    }
    outnl();
    comment();
    errsum1();
}

static void errsum1()
{
    outudec(errcount);
    outnstr(" errors detected");
}

/* fatal error, exit early */
void fatalerror(char *message)
{
    error(message);
    finishup();
}

/* finish up compile */
void finishup()
{
    if (!orig_cppmode)
    {
        if (watchlc)
        {
            cseg();
            outop0str("if *-.program.start-");
            outnhex(getlc());
            outfail();
            outnstr("phase error");
            outop0str("endif\n");
        }
#ifdef HOLDSTRINGS
        dumpstrings();
#endif
        dumpglbs();
        errorsummary();
    }
    closein();
    closeout();
    exit(errcount == 0 ? 0 : 1);
}

/* flush output file */
void flushout()
{
    unsigned nbytes;

    switch (outstage)
    {
        case 0:
            nbytes = (unsigned)(outbufptr - outbuf);
            outbufptr = outbuf;
            break;
        case 2:
            nbytes = OUTBUFSIZE;
            outbufptr = outbuf;
            outbuftop = outbufmid;
            outstage = 3;
            break;
        default:
            nbytes = OUTBUFSIZE;
            if (outstage == 1)
            {
                nbytes = 0;
            }
            outbufptr = outbufmid;
            outbuftop = outbufend;
            outstage = 2;
            break;
    }
    if (nbytes != 0)
    {
        if (!orig_cppmode)
        {
            clearlabels(outbufptr, outbufptr + nbytes);
        }
        if (write(output, outbufptr, nbytes) != nbytes)
        {
            write(2, "output file error\n", 18);
            closein();
            close(output);
            exit(1);
        }
    }
}

void initout()
{
    static char smallbuf[1];

    outbufend = outbuftop = (outbuf = outbufptr = smallbuf) + sizeof smallbuf;
    output = 1;            /* standard output */
}

void limiterror(char *message)
{
    error2error("compiler limit exceeded - ", message);
    finishup();
}

void openout(char *oname)
{
    if (output != 1)
    {
        fatalerror("more than one output file");
    }
    if ((output = creat(oname, CREATPERMS)) < 0)
    {
        output = 1;
        fatalerror("cannot open output");
    }
}

/* print character */
void outbyte(int c)
{
#if C_CODE || __AS09__ + __AS386_16__ + __AS386_32__ != 1
    char *outp;

    outp = outbufptr;
    *outp++ = c;
    outbufptr = outp;
    if (outp >= outbuftop)
    {
        flushout();
    }
#else /* !C_CODE etc */

#if __AS09__
# asm
    TFR    X,D
    LDX    _outbufptr,PC
    STB    ,X+
    STX    _outbufptr,PC
    CMPX    _outbuftop,PC
    LBHS    CALL.FLUSHOUT
# endasm
#endif /* __AS09__ */

#if __AS386_16__
# asm
# if !__FIRST_ARG_IN_AX__
    pop    dx
    pop    ax
    dec    sp
    dec    sp
# else
#  if ARGREG != DREG
    xchg    ax,bx
#  endif
# endif
    mov    bx,[_outbufptr]
    mov    [bx],al
    inc    bx
    mov    [_outbufptr],bx
    cmp    bx,[_outbuftop]
    jae    Outbyte.Flush
# if !__FIRST_ARG_IN_AX__
    jmp    dx
# else
    ret
# endif

Outbyte.Flush:
# if !__FIRST_ARG_IN_AX__
    push    dx
# endif
    br    _flushout
# endasm
#endif /* __AS386_16__ */

#if __AS386_32__
# asm
# if !__FIRST_ARG_IN_AX__
    mov    eax,_outbyte.c[esp]
# else
#  if ARGREG != DREG
    xchg    eax,ebx
#  endif
# endif
    mov    ecx,[_outbufptr]
    mov    [ecx],al
    inc    ecx
    mov    [_outbufptr],ecx
    cmp    ecx,[_outbuftop]
    jae    Outbyte.Flush
    ret

Outbyte.Flush:
    br    _flushout
# endasm
#endif /* __AS386_32__ */
#endif /* C_CODE etc */
}

/* print comma */
void outcomma()
{
    outbyte(',');
}

/* print line number in format ("# %u \"%s\"%s", nr, fname, str) */
void outcpplinenumber(unsigned nr, char *fname, char *str)
{
    outstr("# ");
    outudec(nr);
    outstr(" \"");
    outstr(fname);
    outstr("\"");
    outnstr(str);
}

/* print unsigned offset, hex format */
void outhex(uoffset_T num)
{
#ifdef HEXSTARTCHAR
    if (num >= 10)
    {
        outbyte(HEXSTARTCHAR);
    }
#endif
    outhexdigs(num);
#ifdef HEXENDCHAR
    if (num >= 10)
    outbyte(HEXENDCHAR);
#endif
}

/* print unsigned offset, hex format with digits only (no hex designator) */
void outhexdigs(uoffset_T num)
{
    if (num >= 0x10)
    {
        outhexdigs(num / 0x10);
        num %= 0x10;
    }
    outbyte(hexdigits[(int)num]);
}

/* print string terminated by EOL */
void outline(char *s)
{
    char *outp;
    char *rs;

    outp = outbufptr;
    rs = s;
#ifdef COEOL
    if (*rs == COEOL)
    ++rs;
#endif
    do
    {
        *outp++ = *rs;
        if (outp >= outbuftop)
        {
            outbufptr = outp;
            flushout();
            outp = outbufptr;
        }
    } while (*rs++ != EOL);
    outbufptr = outp;
#ifdef COEOL
    if (*rs == COEOL)
    outbyte(COEOL);
#endif
}

/* print minus sign */
void outminus()
{
    outbyte('-');
}

/* print character, then newline */
void outnbyte(int byte)
{
    outbyte(byte);
    outnl();
}

/* print unsigned offset, hex format, then newline */
void outnhex(uoffset_T num)
{
    outhex(num);
    outnl();
}

/* print newline */
void outnl()
{
    if (watchlc)
    {
        outtab();
        comment();
        outhex(getlc());
    }
    outbyte('\n');
}

/* print opcode and newline, bump lc by 1 */
void outnop1str(char *s)
{
    opcodeleadin();
    outstr(s);
    bumplc();
    outnl();
}

/* print opcode and newline, bump lc by 2 */
void outnop2str(char *s)
{
    opcodeleadin();
    outstr(s);
    bumplc2();
    outnl();
}

/* print string, then newline */
void outnstr(char *s)
{
    outstr(s);
    outnl();
}

/* print opcode */
void outop0str(char *s)
{
    opcodeleadin();
    outstr(s);
}

/* print opcode, bump lc by 1 */
void outop1str(char *s)
{
    opcodeleadin();
    outstr(s);
    bumplc();
}

/* print opcode, bump lc by 2 */
void outop2str(char *s)
{
    opcodeleadin();
    outstr(s);
    bumplc2();
}

/* print opcode, bump lc by 3 */
void outop3str(char *s)
{
    opcodeleadin();
    outstr(s);
    bumplc3();
}

/* print plus sign */
void outplus()
{
    outbyte('+');
}

/* print signed offset, hex format */
void outshex(num)offset_T num;
{
    if (num >= -(maxoffsetto + 1))
    {
        outminus();
        num = -num;
    }
    outhex((uoffset_T)num);
}

/* print string  */
void outstr(s)char *s;
{
#if C_CODE || __AS09__ + __AS386_16__ + __AS386_32__ != 1
    char *outp;
    char *rs;

    outp = outbufptr;
    rs = s;
    while (*rs)
    {
        *outp++ = *rs++;
        if (outp >= outbuftop)
        {
            outbufptr = outp;
            flushout();
            outp = outbufptr;
        }
    }
    outbufptr = outp;
#else /* !C_CODE etc */

#if __AS09__
# asm
    LEAU    ,X
    LDX    _outbuftop,PC
    PSHS    X
    LDX    _outbufptr,PC
    BRA    OUTSTR.NEXT

CALL.FLUSHOUT
    PSHS    U,B
    STX    _outbufptr,PC
    LBSR    _flushout
    LDX    _outbufptr,PC
    LDY    _outbuftop,PC
    PULS    B,U,PC

OUTSTR.LOOP
    STB    ,X+
    CMPX    ,S
    BLO    OUTSTR.NEXT
    BSR    CALL.FLUSHOUT
    STY    ,S
OUTSTR.NEXT
    LDB    ,U+
    BNE    OUTSTR.LOOP
    STX    _outbufptr,PC
    LEAS    2,S
# endasm
#endif /* __AS09__ */

#if __AS386_16__
# asm
# if !__CALLER_SAVES__
    mov    dx,di
    mov    cx,si
# endif
# if !__FIRST_ARG_IN_AX__
    pop    ax
    pop    si
    dec    sp
    dec    sp
    push    ax
# else
#  if ARGREG == DREG
    xchg    si,ax
#  else
    mov    si,bx
#  endif
# endif
    mov    di,[_outbufptr]
    mov    bx,[_outbuftop]
    br    OUTSTR.NEXT

CALL.FLUSHOUT:
    push    si
# if !__CALLER_SAVES__
    push    dx
    push    cx
# endif
    push    ax
    mov    [_outbufptr],di
    call    _flushout
    mov    di,[_outbufptr]
    mov    bx,[_outbuftop]
    pop    ax
# if !__CALLER_SAVES__
    pop    cx
    pop    dx
#endif
    pop    si
    ret

OUTSTR.LOOP:
    stosb
    cmp    di,bx
    jb    OUTSTR.NEXT
    call    CALL.FLUSHOUT
OUTSTR.NEXT:
    lodsb
    test    al,al
    jne    OUTSTR.LOOP
    mov    [_outbufptr],di
# if !__CALLER_SAVES__
    mov    si,cx
    mov    di,dx
# endif
# endasm
#endif /* __AS386_16__ */

#if __AS386_32__
# asm
# if !__CALLER_SAVES__
    mov    edx,edi
    push    esi
#  define TEMPS 4
# else
#  define TEMPS 0
# endif
# if !__FIRST_ARG_IN_AX__
    mov    esi,TEMPS+_outstr.s[esp]
# else
#  if ARGREG == DREG
    xchg    esi,eax
#  else
    mov    esi,ebx
#  endif
# endif
    mov    edi,[_outbufptr]
    mov    ecx,[_outbuftop]
    br    OUTSTR.NEXT

CALL.FLUSHOUT:
    push    esi
# if !__CALLER_SAVES__
    push    edx
# endif
    push    eax
    mov    [_outbufptr],edi
    call    _flushout
    mov    edi,[_outbufptr]
    mov    ecx,[_outbuftop]
    pop    eax
# if !__CALLER_SAVES__
    pop    edx
# endif
    pop    esi
    ret

OUTSTR.LOOP:
    stosb
    cmp    edi,ecx
    jb    OUTSTR.NEXT
    call    CALL.FLUSHOUT
OUTSTR.NEXT:
    lodsb
    test    al,al
    jne    OUTSTR.LOOP
    mov    [_outbufptr],edi
# if !__CALLER_SAVES__
    pop    esi
    mov    edi,edx
# endif
# endasm
#endif /* __AS386_32__ */
#endif /* C_CODE etc */
}

/* print tab */
void outtab()
{
    outbyte('\t');
}

/* print unsigned, decimal format */
void outudec(unsigned num)
{
    char str[10 + 1];

    str[sizeof str - 1] = 0;
    outstr(pushudec(str + sizeof str - 1, num));
}

#ifdef MC6809
#ifdef DEBUG

/* print unsigned value, hex format (like outhex except value_t is larger) */

void outuvalue(num)
uvalue_t num;
{
#ifdef HEXSTARTCHAR
    if (num >= 10)
    outbyte(HEXSTARTCHAR);
#endif
    outvaldigs(num);
#ifdef HEXENDCHAR
    if (num >= 10)
    outbyte(HEXENDCHAR);
#endif
}

/* print unsigned value, hex format with digits only (no hex designator) */
static void outvaldigs(uvalue_t num)
{
    if (num >= 0x10)
    {
    outvaldigs(num / 0x10);
    num %= 0x10;
    }
    outbyte(hexdigits[(int32_t) num]);
}

/* print signed value, hex format (like outshex except value_t is larger) */
void outvalue(value_t num)
{
    if (num < 0)
    {
    outminus();
    num = -num;
    }
    outuvalue((uoffset_T) num);
}

#endif /* DEBUG */
#endif /* MC6809 */

/* push decimal digits of an unsigned onto a stack of chars */
char *pushudec(char *s, register unsigned num)
{
    register unsigned reduction;

    while (num >= 10)
    {
        reduction = num / 10;
        *--s = num - 10 * reduction + '0';
        num = reduction;
    }
    *--s = num + '0';
    return s;
}

void setoutbufs()
{
    if (!isatty(output))
    {
        outbufptr = outbuf = ourmalloc(2 * OUTBUFSIZE);
#ifdef TS
        ts_s_outputbuf += 2 * OUTBUFSIZE;
#endif
        outbufmid = outbuftop = outbufptr + OUTBUFSIZE;
        outbufend = outbufmid + OUTBUFSIZE;
        outstage = 1;
    }
    if (watchlc)
    {
        outnstr(".program.start:\n");
    }    /* kludge temp label */
}
