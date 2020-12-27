/* scan.c - lexical analyser for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#include <stdlib.h>

#include <swancc.h>
#include <swancc/input.h>
#include <swancc/os.h>
#include <swancc/output.h>
#include <swancc/sc.h>
#include <swancc/sizes.h>
#include <swancc/table.h>
#include <swancc/type.h>
#include <swancc/scan.h>
#include <swancc/preproc.h>

/* Global variables */
op_t arg1op;        /* LISTOP, or ROOTLISTOP if arg1inreg */
struct constant_t constant;            /* value of last constant scanned */
char funcname[NAMESIZE];    /* name of current function for unique labels */
char gs2name[2 + NAMESIZE]; /* 2 reserved for namespace keys */
struct symstruct *gsymptr;  /* symbol ptr for last identifier */
bool_t incppexpr;           /* nonzero while scanning cpp expression */
sym_t sym;                  /* current symbol */
sym_t symofchar[];          /* table to convert chars to their symbols */
bool_t expect_statement;    /* If set #asm needs to clear the recursive pending operations. ie: if stmts. */

#define GCH1() do { if (SYMOFCHAR(ch = *++lineptr) == SPECIALCHAR) specialchar(); } while (0)

sym_t symofchar[256] = {
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    WHITESPACE,
    SPECIALCHAR,
    BADCHAR,
    WHITESPACE,
    SPECIALCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,

    WHITESPACE,
    LOGNOTOP,
    STRINGCONST,
    CONTROL,    /*  !"# */
    BADCHAR,
    MODOP,
    AMPERSAND,
    CHARCONST,    /* $%&' */
    LPAREN,
    RPAREN,
    STAR,
    ADDOP,        /* ()*+ */
    COMMA,
    HYPHEN,
    FLOATCONST,
    SLASH,        /* ,-./ */

    INTCONST,
    INTCONST,
    INTCONST,
    INTCONST,    /* 0123 */
    INTCONST,
    INTCONST,
    INTCONST,
    INTCONST,    /* 4567 */
    INTCONST,
    INTCONST,
    COLON,
    SEMICOLON,    /* 89:; */
    LTOP,
    ASSIGNOP,
    GTOP,
    CONDOP,        /* <=>? */

    BADCHAR,
    IDENT,
    IDENT,
    IDENT,        /* @ABC */
    IDENT,
    IDENT,
    IDENT,
    IDENT,            /* DEFG */
    IDENT,
    IDENT,
    IDENT,
    IDENT,            /* HIJK */
    IDENT,
    IDENT,
    IDENT,
    IDENT,            /* LMNO */
    IDENT,
    IDENT,
    IDENT,
    IDENT,            /* PQRS */
    IDENT,
    IDENT,
    IDENT,
    IDENT,            /* TUVW */
    IDENT,
    IDENT,
    IDENT,
    LBRACKET,        /* XYZ[ */
    SPECIALCHAR,
    RBRACKET,
    EOROP,
    IDENT,    /* \]^_ */

    BADCHAR,
    IDENT,
    IDENT,
    IDENT,        /* `abc */
    IDENT,
    IDENT,
    IDENT,
    IDENT,            /* defg */
    IDENT,
    IDENT,
    IDENT,
    IDENT,            /* hijk */
    IDENT,
    IDENT,
    IDENT,
    IDENT,            /* lmno */
    IDENT,
    IDENT,
    IDENT,
    IDENT,            /* pqrs */
    IDENT,
    IDENT,
    IDENT,
    IDENT,            /* tuvw */
    IDENT,
    IDENT,
    IDENT,
    LBRACE,        /* xyz{ */
    OROP,
    RBRACE,
    NOTOP,
    BADCHAR,        /* |}~  */

    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,

    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,

    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,

    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR,
    BADCHAR
};

static void getident(void);

static void intconst(void);

static void ppnumber(void);

static void newstringorcharconst(void);

/* skip white space up to end of line (skip EOL's if in comments) */
void blanks()
{
    char *reglineptr;

    while (TRUE)
    {
        reglineptr = lineptr;
        while (SYMOFCHAR(*reglineptr) == WHITESPACE)
        {
            ++reglineptr;
        }
        lineptr = reglineptr;
        if (SYMOFCHAR(ch = *reglineptr) == SPECIALCHAR)
        {
            specialchar();
            if (ch == EOL)
            {
                return;
            }
            if (SYMOFCHAR(ch) == WHITESPACE)
            {
                continue;
            }
            reglineptr = lineptr;
        }
        if (*reglineptr != '/')
        {
            return;
        }
        if (SYMOFCHAR(*(reglineptr + 1)) == SPECIALCHAR)
        {
            ch = *(lineptr = reglineptr + 1);
            specialchar();
            ch = *(reglineptr = --lineptr) = '/';    /* pushback */
        }
        if (*(reglineptr + 1) != '*')
        {
            return;
        }
        gch1();
        skipcomment();
    }
}

void cppscan(int asm_only)
{
    int start_of_line = 1;
#ifndef ASM_BARE
    virtual_nl = 1;
#endif
    while (!asm_only || asmmode)
    {
        int sym = SYMOFCHAR(ch);

        switch (sym)
        {
            case CONTROL:
                if (!start_of_line)
                {
                    sym = BADCHAR;
                }
                break;
            case WHITESPACE:
                break;
            case SPECIALCHAR:
                if (ch == '\\')
                {
                    sym = BADCHAR;
                }
                start_of_line = 1;
                break;
            case CHARCONST:
            case STRINGCONST:
                if (asmmode)
                {
                    sym = BADCHAR;
                }
            default:
                start_of_line = 0;
        }

        switch (sym)
        {
            case CHARCONST:
            case STRINGCONST:
                stringorcharconst();
                *charptr = 0;    /* room reserved */
                outstr(constant.value.s);    /* XXX - embedded null would kill it */
                charptr = constant.value.s;
                break;
            case CONTROL:
                gch1();
                if (maclevel != 0)
                {
                    error("# in a macro: not preprocessed");    /* XXX? */
                    break;
                }
                else
                {
                    docontrol();
#ifndef ASM_BARE
                    virtual_nl = 1;
#endif
                    continue;
                }
            case SLASH:
                gch1();
                if (ch == '*')
                {
                    skipcomment();
                    break;
                }
                outbyte('/');
                break;
            case FLOATCONST:
                gch1();
                if (SYMOFCHAR(ch) == INTCONST)
                {
                    --lineptr;
                    ch = '.';    /* pushback */
                    ppnumber();
                    outstr(constant.value.s);
                    charptr = constant.value.s;
                }
                else
                {
                    outbyte('.');
                }
                break;
            case IDENT:
                getident();
                if ((gsymptr = findlorg(gsname)) != NULL)
                {
                    if (gsymptr->flags == DEFINITION)
                    {
                        entermac();
                        break;
                    }
                }
                outstr(gsname);
                break;
            case INTCONST:
                ppnumber();
                outstr(constant.value.s);
                charptr = constant.value.s;
                break;
            case SPECIALCHAR:
                specialchar();    /* XXX - this is necessary because entermac
                 * and skipeol can leave lineptr at a special
                 * char although GCH1 cannot.  entermac would
                 * be easy to fix, but too many places already
                 * have to handle the extra work after skipeol
                 * for other reasons.
                 */
                if (ch == EOL)
                {
                    skipeol();
                    if (eofile)
                    {
                        return;
                    }
                    break;
                }
#ifdef COEOL
                if (ch == COEOL)
                {
                gch1();
                break;
                }
#endif
                if (SYMOFCHAR(ch) != SPECIALCHAR)
                {
                    break;
                }        /* specialchar advanced the input */
                /* must be '\\' */
            default:
                /* Allow for multi-instruction lines in asm */
                if (ch == '^' && !orig_cppmode && asmmode)
                    ch = '\n';

                OUTBYTE(ch);
                ch = *++lineptr;
                if (SYMOFCHAR(ch) == SPECIALCHAR && ch != '\\')
                    specialchar();
                break;
        }
#ifndef ASM_BARE
        virtual_nl = 0;
#endif
    }
}

void eofin(char *message)
{
    error2error("end of file in ", message);
}

/* get identifier in gsname == gs2name + 2 */
static void getident()
{
    int c;
    char *idptr;
    int length;
    char *reglineptr;

    idptr = gsname;
    length = NAMESIZE;
    reglineptr = lineptr;
    c = ch;
    more:
    do
    {
        *idptr++ = c;
        c = (uint8_t)*++reglineptr;
    } while (symofchar[c] <= MAXIDSYM && --length != 0);
    ch = c;
    *idptr = 0;
    lineptr = reglineptr;
    if (symofchar[c] == SPECIALCHAR)
    {
        specialchar();
        if (SYMOFCHAR(ch) <= MAXIDSYM && length != 0)
        {
            reglineptr = lineptr;
            c = ch;
            goto more;
        }
    }
}

/* return nonzero if at an identifier */
bool_t isident()
{
    if (SYMOFCHAR(ch) != IDENT)
    {
        return FALSE;
    }
    getident();
    return TRUE;
}

static void intconst()
{
    int32_t base;
    int32_t digit;
    char *digptr;
    int32_t lcount;
    int32_t ucount;
    bool_t dflag;

    ppnumber();
    digptr = charptr = constant.value.s;
    if (*digptr == '0')
    {
        if (digptr[1] != 'x' && digptr[1] != 'X')
        {
            base = 8;
        }
        else
        {
            base = 16;
            digptr += 2;
        }
    }
    else
    {
        base = 10;
    }
    dflag = FALSE;
    if (*digptr == 0)
    {
        error("constant has no digits");
    }
    if (base != 16)
    {
        while (SYMOFCHAR(*digptr) == INTCONST)
        {
            ++digptr;
        }
        if (*digptr == '.')
        {
            dflag = TRUE;
            ++digptr;
            while (SYMOFCHAR(*digptr) == INTCONST)
            {
                ++digptr;
            }
        }
        if (*digptr == 'E' || *digptr == 'e')
        {
            dflag = TRUE;
            ++digptr;
            if (*digptr == '+' || *digptr == '-')
            {
                ++digptr;
            }
            if (SYMOFCHAR(*digptr) != INTCONST)
            {
                error("exponent has no digits");
            }
            while (SYMOFCHAR(*digptr) == INTCONST)
            {
                ++digptr;
            }
        }
    }
    if (dflag)
    {
        constant.type = dtype;
        digit = *digptr;
        *digptr = 0;
        constant.value.d = atof(constant.value.s);
        *digptr = digit;
        if (digit == 'F' || digit == 'f')
        {
            ++digptr;
            constant.type = fltype;
            constant.value.d = (float)constant.value.d;
        }
        sym = FLOATCONST;
        if (*digptr != 0)
        {
            error("junk at end of floating point constant");
        }
        return;
    }
    if (base != 16)
    {
        digptr = constant.value.s;
    }
    constant.value.v = 0;
    while (*digptr >= '0')
    {
        digit = *digptr;
        if (digit > '9')
        {
            if (base != 16)
            {
                break;
            }
            if (digit >= 'a' && digit <= 'f')
            {
                digit = digit + (-'a' + '0' + 10);
            }
            else if (digit >= 'A' && digit <= 'F')
            {
                digit = digit + (-'A' + '0' + 10);
            }
            else
            {
                break;
            }
        }
        constant.value.v = base * constant.value.v + (digit - '0');
        ++digptr;
    }
    ucount = lcount = 0;
    while (TRUE)
    {
        if (*digptr == 'L' || *digptr == 'l')
        {
            ++lcount;
        }
        else if (*digptr == 'U' || *digptr == 'u')
        {
            ++ucount;
        }
        else
        {
            break;
        }
        ++digptr;
    }
    if (constant.value.v <= MAX_INT && lcount == 0 && ucount == 0)
    {
        constant.type = itype;
    }
    else if (constant.value.v <= MAX_UINT && lcount == 0 && (base != 10 || ucount != 0))
    {
        constant.type = uitype;
    }
    else if (constant.value.v <= MAX_LONG && ucount == 0)
    {
        constant.type = ltype;
    }
    else
    {
        constant.type = ultype;
    }

    if (lcount > 1 || ucount > 1 || *digptr != 0)
    {
        error("junk at end of integer constant");
    }
}

void nextsym()
{
    static char lastch;
    char *reglineptr;

    if (expect_statement && asmmode)
    {
        outnstr("!BCC_ASM");
        dumplocs();
        cppscan(1);
        outnstr("!BCC_ENDASM");
    }
    else
    {
        while (TRUE)        /* exit with short, fast returns */
        {
            reglineptr = lineptr;
            while ((sym = SYMOFCHAR(*reglineptr)) == WHITESPACE)
            {
                ++reglineptr;
            }
            lastch = *reglineptr;
            ch = *(lineptr = reglineptr + 1);
            if (SYMOFCHAR(ch) == SPECIALCHAR && sym != SPECIALCHAR)
            {
                specialchar();
            }
            switch (sym)
            {
                case CHARCONST:
                    --lineptr;
                    ch = lastch;    /* pushback */
                    stringorcharconst();
                    newstringorcharconst();
                    if (charptr == constant.value.s)
                    {
                        charptr = constant.value.s;
                        error("empty character constant");
                        constant.value.v = 0;
                    }
                    else
                    {
                        if (charptr != constant.value.s + 1)
                        {
                            error("character constant too long");
                        }
                        charptr = constant.value.s;
                        constant.value.v = *constant.value.s & CHMASKTO;
                    }
                    constant.type = itype;
                    return;
                case CONTROL:
                    if (maclevel != 0)
                    {
                        error("# in a macro: not preprocessed");    /* XXX? */
                        return;
                    }
                    else
                    {
                        int old_asmmode = asmmode;
                        docontrol();
                        if (asmmode && !old_asmmode)
                        {
                            sym = SEMICOLON;
                            return;
                        }
                        break;
                    }
                case FLOATCONST:
                    if (SYMOFCHAR(ch) == INTCONST)
                    {
                        --lineptr;
                        ch = lastch;
                        intconst();
                    }
                    else
                    {
                        sym = STRUCELTOP;
                    }
                    return;
                case IDENT:
                    --lineptr;
                    ch = lastch;
                    getident();
                    if ((gsymptr = findlorg(gsname)) != NULL)
                    {
                        if (gsymptr->flags == DEFINITION)
                        {
                            entermac();
                            break;
                        }
                        if (gsymptr->flags == KEYWORD)
                        {
                            sym = gsymptr->offset.offsym;
                        }
                    }
                    return;
                case INTCONST:
                    --lineptr;
                    ch = lastch;
                    intconst();
                    return;
                case SPECIALCHAR:
                    ch = *(lineptr = reglineptr);
                    specialchar();
                    if (ch == EOL)
                    {
                        if (incppexpr)
                        {
                            sym = SEMICOLON;
                            return;
                        }
                        skipeol();
                        if (eofile)
                        {
                            sym = EOFSYM;
                            return;
                        }
                        break;
                    }
                    if (SYMOFCHAR(ch) != SPECIALCHAR)
                    {
                        break;
                    }
                    gch1();
                    /* fall through to BADCHAR case */
                case BADCHAR:
                    error("illegal character");
                    break;
                case STRINGCONST:
                    --lineptr;
                    ch = lastch;    /* pushback */
                    stringorcharconst();
                    newstringorcharconst();
#ifdef TS
                    ts_n_string += charptr - constant.value.s;
#endif
                    return;
                case COLON:        /* could be COLONOP or in label */
                case COMMA:        /* could be COMMAOP or in list */
                case CONDOP:
                case LBRACE:
                case LBRACKET:
                case LPAREN:
                case NOTOP:
                case RBRACE:
                case RBRACKET:
                case RPAREN:
                case SEMICOLON:
                    return;
                case ASSIGNOP:
                    if (ch == '=')
                    {
                        sym = EQOP;
                        gch1();
                    }
                    return;
                case ADDOP:
                    if (ch == '+')
                    {
                        sym = INCSYM;    /* PREINCOP or POSINCOP */
                        gch1();
                    }
                    else if (ch == '=')
                    {
                        sym = ADDABOP;
                        gch1();
                    }
                    return;
                case HYPHEN:        /* NEGOP or SUBOP or decoded below */
                    if (ch == '-')
                    {
                        sym = DECSYM;    /* PREDECOP or POSTDECOP */
                        gch1();
                    }
                    else if (ch == '=')
                    {
                        sym = SUBABOP;
                        gch1();
                    }
                    else if (ch == '>')
                    {
                        sym = STRUCPTROP;
                        gch1();
                    }
                    return;
                case STAR:        /* INDIRECTOP or MULOP or as below */
                    if (ch == '=')
                    {
                        sym = MULABOP;
                        gch1();
                    }
                    return;
                case SLASH:
                    if (ch == '*')
                    {
                        skipcomment();
                        break;
                    }
                    if (ch == '=')
                    {
                        sym = DIVABOP;
                        gch1();
                    }
                    else
                    {
                        sym = DIVOP;
                    }
                    return;
                case MODOP:
                    if (ch == '=')
                    {
                        sym = MODABOP;
                        gch1();
                    }
                    return;
                case LTOP:
                    if (ch == '=')
                    {
                        sym = LEOP;
                        gch1();
                    }
                    else if (ch == '<')
                    {
                        gch1();
                        if (ch == '=')
                        {
                            sym = SLABOP;
                            gch1();
                        }
                        else
                        {
                            sym = SLOP;
                        }
                    }
                    return;
                case GTOP:
                    if (ch == '=')
                    {
                        sym = GEOP;
                        gch1();
                    }
                    else if (ch == '>')
                    {
                        gch1();
                        if (ch == '=')
                        {
                            sym = SRABOP;
                            gch1();
                        }
                        else
                        {
                            sym = SROP;
                        }
                    }
                    return;
                case AMPERSAND:        /* ADDRESSOP or ANDOP or as below */
                    if (ch == '&')
                    {
                        sym = LOGANDOP;
                        gch1();
                    }
                    else if (ch == '=')
                    {
                        sym = ANDABOP;
                        gch1();
                    }
                    return;
                case EOROP:
                    if (ch == '=')
                    {
                        sym = EORABOP;
                        gch1();
                    }
                    return;
                case OROP:
                    if (ch == '|')
                    {
                        sym = LOGOROP;
                        gch1();
                    }
                    else if (ch == '=')
                    {
                        sym = ORABOP;
                        gch1();
                    }
                    return;
                case LOGNOTOP:
                    if (ch == '=')
                    {
                        sym = NEOP;
                        gch1();
                    }
                    return;

                default:
                    //fatalerror("Invalid symbol");
                    break;
            }
        }
    }
}

static void ppnumber()
{
    constant.value.s = charptr;
    while (TRUE)
    {
        if (charptr >= char1top)
        {
            constant.value.s = growobject(constant.value.s, 2);
        }
        *charptr++ = ch;
        if (ch == 'E' || ch == 'e')
        {
            gch1();
            if (ch == '+' || ch == '-')
            {
                continue;
            }
        }
        else
            GCH1();
        if (SYMOFCHAR(ch) > MAXPPNUMSYM)
        {
            break;
        }
    }
    *charptr = 0;
}

void stringorcharconst()
{
    char terminator;

    constant.value.s = charptr;
    terminator = ch;
    while (TRUE)
    {
        if (charptr >= char3top)
        {
            constant.value.s = growobject(constant.value.s, 4);
        }
        *charptr++ = ch;
        if (ch == '\\')
        {
            gch1();
            if (ch == EOL)
            {
                if (!orig_cppmode)
                {
                    eofin("escape sequence");
                }
                break;
            }
            *charptr++ = ch;
        }
        GCH1();
        if (ch == EOL)
        {
            if (!orig_cppmode && ifcheck())
            {
                error(terminator == '"' ? "end of line in string constant" : "end of line in character constant");
            }
            break;
        }
        if (ch == terminator)
        {
            *charptr++ = terminator;

            /* This adds ansi string concatenation BUT only on one line */
            do
            {
                ch = *++lineptr;
            } while (SYMOFCHAR(ch) == WHITESPACE);
            if (ch == terminator) /* Hang on .. that's another string ... */
            {
                charptr--;       /* Stick it together */
                gch1();
            }
            else
            {
                break;
            }
        }
    }
}

static void newstringorcharconst()
{
    char *endinptr;
    int escvalue;
    int32_t maxdigits;
    char *inptr;
    char *outptr;

    outptr = constant.value.s;
    inptr = charptr - 1;
    if (inptr == outptr || *inptr != *outptr)
    {
        endinptr = charptr;    /* not properly terminated */
    }
    else
    {
        endinptr = inptr;
    }
    inptr = outptr + 1;
    while (inptr != endinptr)
    {
        if (*inptr == '\\')
        {
            if (++inptr == endinptr)
            {
                break;
            }        /* error reported already */
            if (*inptr >= '0' && *inptr < '8')
            {
                escvalue = 0;
                maxdigits = 3;
                do
                {
                    escvalue = 8 * escvalue + *inptr - '0';
                    ++inptr;
                } while (--maxdigits != 0 && *inptr >= '0' && *inptr < '8');
                --inptr;
                *outptr++ = escvalue;
            }
            else
                switch (*inptr)
                {
                    case '"':
                    case '\'':
                    case '\\':
                        *outptr++ = *inptr;
                        break;
                    case 'a':
                        *outptr++ = 7;
                        break;
                    case 'b':
                        *outptr++ = 8;
                        break;
                    case 'f':
                        *outptr++ = 12;
                        break;
                    case 'n':
                        *outptr++ = EOLTO;
                        break;
                    case 'r':
                        *outptr++ = 13;
                        break;
                    case 't':
                        *outptr++ = 9;
                        break;
                    case 'v':
                        *outptr++ = 11;
                        break;
                    default:
                        error("%wnonstandard escape sequence");
                        *outptr++ = *inptr;
                        break;
                }
        }
        else
        {
            *outptr++ = *inptr;
        }
        ++inptr;
    }
    charptr = outptr;
}
