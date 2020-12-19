/* preprocessor routines for swancc
 *
 * swancc: A rudimentary C compiler for the WonderSwan
 *
 * Based on bcc 0.16.2 by Bruce Evans
 *
 * Copyright (C) 1992 Bruce Evans
 * Copyright (C) 2020 Manoël <godzil> Trapier / 986-Studio
 */

#include <string.h>

#include <swancc.h>
#include <swancc/input.h>
#include <swancc/os.h>
#include <swancc/output.h>
#include <swancc/parser.h>
#include <swancc/sc.h>
#include <swancc/scan.h>
#include <swancc/table.h>
#include <swancc/type.h>
#include <swancc/preproc.h>
#include <swancc/codefrag.h>
#include <swancc/loadexp.h>

#define MAX_IF        32
#define MAX__LINE__    10    /* enough for 32-bit source unsigneds */
#define MAX_MACRO    32
#define MAX_PARAM    127    /* max char with no sign on all machines */

/* Definition types.  These are kept in the 'storage' member of struct
 * symstruct and must be distinct from 'LOCAL' because dumplocs() doesn't
 * check.
 */
enum
{
    DEF_LINE,            /* __LINE__ keyword */
    DEF_NONE            /* nothing special */
};

struct ifstruct
{
    bool_t elseflag;
    bool_t ifflag;
};

struct macroposition
{
    char *maclineptr;
    char **paramlist;
    char *paramspot;
    bool_t inparam;
    indn_t nparam;
};

static char dummyparam[] = {EOL, 0};
static int32_t iflevel;    /* depends on zero init */
static struct ifstruct ifstate;
/* elseflag depends on zero init */
static struct ifstruct ifstack[MAX_IF];

static struct macroposition macrostack[MAX_MACRO];

static void asmcontrol(void);

static void control(void);

static void defineorundefinestring(char *str, bool_t defineflag);

static void elsecontrol(void);

static void endif(void);

static uint32_t getparnames(void);

static void ifcontrol(sym_t ifcase);

static void undef(void);

/* asmcontrol() - process #asm */

static void asmcontrol()
{
#ifdef ASM_BARE
    char treasure;        /* to save at least one leading blank */
#endif

    asmmode = TRUE;
    if (expect_statement)
    {
        return;
    }

    if (orig_cppmode)
    {
        outnstr("#asm");
    }
    else
    {
        outnstr("!BCC_ASM");
        dumplocs();
    }
#ifndef ASM_BARE
    cppscan(1);
#else
    while (TRUE)
    {
    skipline();
    skipeol();
    if (eofile)
    {
        eofin("#asm");
        break;
    }
    if (SYMOFCHAR(ch) == SPECIALCHAR)
        specialchar();
    treasure = 0;
    if (SYMOFCHAR(ch) == WHITESPACE)
        treasure = ch;
    blanks();
    if (ch == '#')
    {
        if (ctext)
        {
        char *lptr;

        comment();
        if (treasure != 0)
            outbyte(treasure);
        lptr = lineptr;
        while (*lptr++ != EOL)    /* XXX - handle COEOL too */
            outbyte(ch);
        outnl();
        }
        gch1();
        docontrol();
        if (!asmmode)
        break;
    }
    else
    {
        if (treasure != 0)
        outbyte(treasure);
        while (ch != EOL)    /* XXX - handle COEOL too */
        {
        outbyte(ch);
        gch1();
        }
        outnl();
    }
    }
#endif
    if (orig_cppmode)
    {
        outnstr("#endasm");
    }
    else
    {
        outnstr("!BCC_ENDASM");
    }
}

/* blanksident() - return nonzero if at blanks followed by an identifier */

bool_t blanksident()
{
    blanks();
    return isident();
}

void checknotinif()
{
    while (iflevel != 0)
    {
        if (ifstate.ifflag)
        {
            eofin("true #conditional");
        }
        else
        {
            eofin("false #conditional");
        }
        endif();
    }
}

/* control() - select and switch to control statement */

static void control()
{
    char sname[NAMESIZE + 1];
    sym_t ctlcase;
    struct symstruct *symptr;
    if (ctext && asmmode)
    {
        comment();
        outudec(input.linenumber);
        outbyte(' ');
        outline(lineptr);
    }

    sname[0] = '#';        /* prepare for bad control */
    sname[1] = 0;
    while (blanksident())
    {
        if ((gsymptr = findlorg(gsname)) == NULL || gsymptr->flags != DEFINITION)
        {
            strcat(sname, gsname);
            break;
        }
        entermac();
    }
    if (sname[1] == 0 && ch == EOL)
    {
        return;
    }
    if (SYMOFCHAR(ch) == INTCONST)
    {
        linecontol();
        return;
    }
    if ((symptr = findlorg(sname)) == NULL)
    {
        if (ifstate.ifflag)
        {
            error(" bad control");
        }
        return;
    }
    ctlcase = symptr->offset.offsym;
    if (ifstate.ifflag == FALSE && (ctlcase < ELSECNTL || ctlcase > IFNDEFCNTL))
    {
        return;
    }
    switch (ctlcase)
    {
        case ASMCNTL:
            if (asmmode)
            {
                if (ifstate.ifflag)
                {
                    error(" bad control");
                }
            }
            else
            {
                asmcontrol();
            }
            break;
        case DEFINECNTL:
            define();
            break;
        case ELSECNTL:
            elsecontrol();
            break;
        case ENDASMCNTL:
            if (!asmmode)
            {
                if (ifstate.ifflag)
                {
                    error(" bad control");
                }
            }
            asmmode = FALSE;
            break;
        case ENDIFCNTL:
            endif();
            break;
        case IFCNTL:
        case IFDEFCNTL:
        case IFNDEFCNTL:
            ifcontrol(symptr->offset.offsym);
            break;
        case INCLUDECNTL:
            include();
            break;
        case LINECNTL:
        {
            linecontol();
            break;
        }
        case UNDEFCNTL:
            undef();
            break;
    }
}

/* define() - process #define */

/*
 * MACRO storage.
 * A symbol recording the macro name is added to the symbol table.
 * This overrides all current scopes of the name (put at head of hash chain).
 * The flags are set to DEFINITION.
 * The indcount is 0 if no parameters, else 1 + number of parameters.
 * The offset field points to the macro string.
 * The macro string is null-terminated but EOL-sentineled.
 * It consists of EOL-terminated substrings followed by parameter numbers,
 * e.g., junk(x,y)=-x+y is stored as '-', EOL, 1, '+', EOL, 2, EOL, 0.
 * Here  1  is for the 1st parameter (start at 1 so 0 can terminate).
 * EOL acts as a sentinel for the scanner.
 * This choice works well because EOL cannot occur in a macro string.
 */
void define()
{
    char sname[NAMESIZE];
    char quote;
    struct symstruct **hashptr;
    struct symstruct *locmark = NULL; /* for -Wall */
    char *macstring;
    uint32_t nparnames;
    char *oldstring;
    struct symstruct *symptr;

    if (!blanksident())
    {
        error("illegal macro name");
        return;
    }
    strcpy(sname, gsname);    /* will overwrite gsname if parameters */
    if (ch == '(')
    {
        locmark = locptr;
        newlevel();        /* temp storage for parameter names */
        nparnames = getparnames() + 1;
    }
    else
    {
        nparnames = 0;
    }
    blanks();
    macstring = charptr;
    quote = 0;
    while (ch != EOL)
    {
        if (charptr >= char1top)
        {  /* check room for char and end of string */
            macstring = growobject(macstring, 2);
        }
        if (nparnames && isident())
        {
            if ((symptr = findlorg(gsname)) != NULL && symptr->level == level)
            {
#ifdef TS
                ++ts_n_macstring_param;
                ts_s_macstring += 2;
#endif
                *charptr++ = EOL;    /* end current string */
                *charptr++ = (char) symptr->indcount;    /* param to insert */
            }
            else
            {
                if (charptr + strlen(gsname) >= chartop)
                {    /* null too */
                    macstring = growobject(macstring, strlen(gsname) + 1);
                }
#ifdef TS
                ++ts_n_macstring_ident;
                ts_s_macstring += strlen(gsname);;
#endif
                strcpy(charptr, gsname);
                charptr += strlen(gsname);    /* discard null */
            }
            continue;
        }
        if (ch == '\\')
        {
            gch1();
            *charptr = '\\';
            *(charptr + 1) = ch;
#ifdef TS
            ++ts_n_macstring_quoted;
            ts_s_macstring += 2;
#endif
            charptr += 2;
            gch1();
            continue;
        }
        if (quote)
        {
            if (ch == quote)
            {
                quote = 0;
            }
        }
        else if (ch == '"' || ch == '\'')
        {
            quote = ch;
        }
        else if (ch == '/')
        {
            if (SYMOFCHAR(*(lineptr + 1)) == SPECIALCHAR)
            {
                gch1();
                ch = *--lineptr = '/';    /* pushback */
            }
            if (*(lineptr + 1) == '*')
            {
                gch1();
                skipcomment();
                /* comment is space in modern cpp's but they have '##' too */
                ch = *--lineptr = ' ';
            }
        }
#ifdef TS
        ++ts_n_macstring_ordinary;
        ++ts_s_macstring;
#endif
        *charptr++ = ch;
        gch1();
    }
    {
        char *rcp;

        /* strip trailing blanks, but watch out for parameters */
        for (rcp = charptr ;
             rcp > macstring && SYMOFCHAR(*(rcp - 1)) == WHITESPACE && (--rcp == macstring || *(rcp - 1) != EOL) ;)
        {
            charptr = rcp;
        }
    }
    if (charptr >= char1top)
    {
        macstring = growobject(macstring, 2);
    }
#ifdef TS
    ++ts_n_macstring_term;
    ts_s_macstring += 2;
#endif
    *charptr++ = EOL;
    *charptr++ = 0;
    if (nparnames)
    {
        oldlevel();
        locptr = locmark;
    }
    /*  if (asmmode) equ(sname, macstring); */

    if ((symptr = findlorg(sname)) != NULL && symptr->flags == DEFINITION)
    {
        if (strcmp(macstring, oldstring = symptr->offset.offp) != 0)
        {
            error("%wredefined macro");
        }
        if (strlen(macstring) > strlen(oldstring = symptr->offset.offp))
        {
            symptr->offset.offp = macstring;
        }
        else
        {
            strcpy(oldstring, macstring);    /* copy if == to avoid test */
            charptr = macstring;
        }
        return;
    }
    symptr = qmalloc(sizeof(struct symstruct) + strlen(sname));
#ifdef TS
    ++ts_n_defines;
    ts_s_defines += sizeof (struct symstruct) + strlen(sname);
#endif
    addsym(sname, vtype, symptr);
    symptr->storage = DEF_NONE;
    symptr->indcount = (indn_t)nparnames;
    symptr->flags = DEFINITION;
    symptr->level = GLBLEVEL;
    symptr->offset.offp = macstring;
    if (*(hashptr = gethashptr(sname)) != NULL)
    {
        symptr->next = *hashptr;
        symptr->next->prev = &symptr->next;
    }
    *hashptr = symptr;
    symptr->prev = hashptr;
}

/* str: "name[=def]" or "name def" */
static void defineorundefinestring(char *str, bool_t defineflag)
{
    char *fakeline;
    uint32_t len;
    bool_t old_eof;

    len = strlen(str);
    strcpy(fakeline = (char *)ourmalloc(3 + len + 2 + 2) + 3, str);
    /* 3 pushback, 2 + 2 guards */
#ifdef TS
    ts_s_fakeline += 3 + len + 2 + 2;
    ts_s_fakeline_tot += 3 + len + 2 + 2;
#endif
    {
        char *endfakeline;

        endfakeline = fakeline + len;
        endfakeline[0] = EOL;    /* guards any trailing backslash */
        endfakeline[1] = EOL;    /* line ends here or before */
    }
    old_eof = eofile;
    eofile = TRUE;            /* valid after first EOL */
    ch = *(lineptr = fakeline);
    if (defineflag)
    {
        if (blanksident())    /* if not, let define() produce error */
        {
            blanks();
            if (ch == '=')
            {
                *lineptr = ' ';
            }
            else if (ch == EOL)
            {
                char *lptr;

                lptr = lineptr;
                lptr[0] = ' ';
                lptr[1] = '1';    /* 2 extra were allocated for this & EOL */
                lptr[2] = EOL;
            }
        }
        ch = *(lineptr = fakeline);
        define();
    }
    else
    {
        undef();
    }
    eofile = old_eof;
#ifdef TS
    ts_s_fakeline_tot -= len + 2 + 2;
#endif
    ourfree(fakeline - 3);
}

/* str: "name[=def]" or "name def" */
void definestring(char *str)
{
    defineorundefinestring(str, TRUE);
}

/* docontrol() - process control statement, loop till "#if" is true */
void docontrol()
{
    while (TRUE)
    {
        control();
        skipline();
        if (ifstate.ifflag)
        {
            return;
        }
        while (TRUE)
        {
            skipeol();
            if (eofile)
            {
                return;
            }
            blanks();
            if (ch == '#')
            {
                gch1();
                break;
            }
            skipline();
        }
    }
}

/* elsecontrol() - process #else */
static void elsecontrol()
{
    if (iflevel == 0)
    {
        error("else without if");
        return;
    }
    ifstate.ifflag = ifstate.elseflag;
    ifstate.elseflag = FALSE;
}

/* endif() - process #endif */
static void endif()
{
    if (iflevel == 0)
    {
        error("endif without if");
        return;
    }
    {
        struct ifstruct *ifptr;

        ifptr = &ifstack[(int)--iflevel];
        ifstate.elseflag = ifptr->elseflag;
        ifstate.ifflag = ifptr->ifflag;
    }
}

/* entermac() - switch line ptr to macro string */
void entermac()
{
    char quote;
    struct symstruct *symptr;
    char **paramhead;
    char **paramlist;
    int ngoodparams;
    int nparleft;
    int lpcount;

    if (maclevel >= MAX_MACRO)
    {
        limiterror("macros nested too deeply (33 levels)");
        return;
    }
    symptr = gsymptr;
    ngoodparams = 0;
    paramhead = NULL;
    if (symptr->indcount != 0)
    {
        nparleft = symptr->indcount - 1;
        if (nparleft == 0)
        {
            paramhead = NULL;
        }
        else
        {
            paramhead = ourmalloc(sizeof *paramlist * nparleft);
        }
        paramlist = paramhead;
#ifdef TS
        ++ts_n_macparam;
        ts_s_macparam += sizeof *paramlist * nparleft;
        ts_s_macparam_tot += sizeof *paramlist * nparleft;
#endif
        blanks();
        while (ch == EOL && !eofile)
        {
            skipeol();
            blanks();
        }
        if (ch != '(')
        {
            if (nparleft > 0)    /* macro has params, doesn't match bare word */
            {
                outstr(symptr->name.namea);
                return;
            }
            error("missing '('");
        }
        else
        {
            gch1();
            while (nparleft)
            {
                --nparleft;
                ++ngoodparams;
                *(paramlist++) = charptr;
                quote = 0;
                lpcount = 1;
                while (TRUE)
                {
                    if (ch == '\\')
                    {
                        gch1();
                        if (charptr >= char1top)
                        {
                            *(paramlist - 1) = growobject(*(paramlist - 1), 2);
                        }
#ifdef TS
                        ++ts_n_macparam_string_quoted;
                        ++ts_s_macparam_string;
                        ++ts_s_macparam_string_tot;
#endif
                        *charptr++ = '\\';
                    }
                    else if (quote)
                    {
                        if (ch == quote)
                        {
                            quote = 0;
                        }
                    }
                    else if (ch == '"' || ch == '\'')
                    {
                        quote = ch;
                    }
                    else if (ch == '/')
                    {
                        if (SYMOFCHAR(*(lineptr + 1)) == SPECIALCHAR)
                        {
                            gch1();
                            ch = *--lineptr = '/';    /* pushback */
                        }
                        if (*(lineptr + 1) == '*')
                        {
                            gch1();
                            skipcomment();
                            ch = *--lineptr = ' ';    /* pushback */
                        }
                    }
                    else if (ch == '(')
                    {
                        ++lpcount;
                    }
                    else if ((ch == ')' && --lpcount == 0) || (ch == ',' && lpcount == 1))
                    {
                        break;
                    }
                    if (ch == EOL)
                    {
                        ch = ' ';
                    }
                    if (charptr >= char1top)
                    {
                        *(paramlist - 1) = growobject(*(paramlist - 1), 2);
                    }
#ifdef TS
                    ++ts_n_macparam_string_ordinary;
                    ++ts_s_macparam_string;
                    ++ts_s_macparam_string_tot;
#endif
                    *charptr++ = ch;
                    if (*lineptr == EOL)
                    {
                        skipeol();    /* macro case disposed of already */
                        if (SYMOFCHAR(ch) == SPECIALCHAR)
                        {
                            specialchar();
                        }
                        if (eofile)
                        {
                            break;
                        }
                    }
                    else
                    {
                        gch1();
                    }
                }
#ifdef TS
                ++ts_n_macparam_string_term;
                ++ts_s_macparam_string;
                ++ts_s_macparam_string_tot;
#endif
                *charptr++ = EOL;
                {
                    char *newparam;
                    char *oldparam;
                    uint32_t size;

                    oldparam = *(paramlist - 1);
                    size = (/* size_t */ uint32_t)(charptr - oldparam);
                    newparam = ourmalloc(size);
#ifdef TS
                    ts_s_macparam_string_alloced += size;
                    ts_s_macparam_string_alloced_tot += size;
#endif
                    memcpy(newparam, oldparam, size);
                    *(paramlist - 1) = newparam;
#ifdef TS
                    ts_s_macparam_string_tot -= charptr - oldparam;
#endif
                    charptr = oldparam;
                }
                if (ch == ',')
                {
                    gch1();
                }
                else
                {
                    break;
                }
            }
        }
        blanks();
        while (ch == EOL && !eofile)
        {
            skipeol();
            blanks();
        }
        if (eofile)
        {
            eofin("macro parameter expansion");
        }
        if (nparleft)
        {
            error("too few macro parameters");
            do
            {
                *(paramlist++) = dummyparam;
            } while (--nparleft);
        }
        if (ch == ')')
        {
            gch1();
        }
        else if (ch == ',')
        {
            error("too many macro parameters");

            /* XXX - should read and discard extra parameters.  Also check
             * for eof at end.
             */
            while (ch != ')')
            {
                if (ch == EOL)
                {
                    skipeol();
                    if (eofile)
                    {
                        break;
                    }
                    continue;
                }
                gch1();
            }
        }
        else
        {
            error("missing ')'");
        }
    }

    if (symptr->storage == DEF_LINE)
    {
        char *str;

        str = pushudec(symptr->offset.offp + MAX__LINE__, input.linenumber);
        memcpy(symptr->offset.offp, str, /* size_t */
               (uint32_t)(symptr->offset.offp + MAX__LINE__ + 1 + 1 - str));
    }

    {
        struct macroposition *mpptr;

        mpptr = &macrostack[maclevel];
        mpptr->paramlist = paramhead;
        mpptr->maclineptr = lineptr;
        ch = *(lineptr = symptr->offset.offp);
        mpptr->inparam = FALSE;
        mpptr->nparam = ngoodparams;
        ++maclevel;
    }
    /*
        comment();
        outstr("MACRO (level ");
        outudec((unsigned) maclevel);
        outstr(") ");
        outline(lineptr);
    */
}

/* getparnames() - get parameter names during macro definition, return count */
static uint32_t getparnames()
{
    uint32_t nparnames;
    struct symstruct *symptr;

    nparnames = 0;
    gch1();
    while (blanksident())
    {
        if ((symptr = findlorg(gsname)) != NULL && symptr->level == level)
        {
            error("repeated parameter");
        }
        symptr = addloc(gsname, itype);
        if (nparnames >= MAX_PARAM)
        {
            limiterror("too many macro parameters (128)");
        }
        else
        {
            ++nparnames;
        }    /* number params from 1 */
        symptr->indcount = nparnames;    /* param number */
        blanks();
        if (ch == ',')
            gch1();
    }
    if (ch != ')')
    {
        error("missing ')'");
    }
    else
    {
        gch1();
    }
    return nparnames;
}

/* ifcontrol - process #if, #ifdef, #ifndef */
static void ifcontrol(sym_t ifcase)
{
    bool_t iftrue;
    struct symstruct *symptr;

    if (iflevel >= MAX_IF)
    {
        limiterror("#if's nested too deeply (33 levels)");
        return;
    }

    {
        struct ifstruct *ifptr;

        ifptr = &ifstack[(int)iflevel++];
        ifptr->elseflag = ifstate.elseflag;
        ifptr->ifflag = ifstate.ifflag;
        ifstate.elseflag = FALSE;    /* prepare for !(if now)||(if future)*/
    }

    if (ifstate.ifflag)
    {
        if ((sym_t)ifcase != IFCNTL)
        {
            iftrue = FALSE;
            if (blanksident() && (symptr = findlorg(gsname)) != NULL && symptr->flags == DEFINITION)
            {
                iftrue = TRUE;
            }
        }
        else
        {
            incppexpr = TRUE;
            nextsym();
            iftrue = constexpression() != 0;
            incppexpr = FALSE;
        }
        if ((!iftrue && (sym_t)ifcase != IFNDEFCNTL) || (iftrue && (sym_t)ifcase == IFNDEFCNTL))
        {
            ifstate.elseflag = TRUE;
            ifstate.ifflag = FALSE;
        }
    }
}

/* ifinit() - initialise if state */
void ifinit()
{
    ifstate.ifflag = TRUE;
}

int ifcheck()
{
    return (ifstate.ifflag == TRUE);
}

/* leavemac() - leave current and further macro substrings till not at end */
void leavemac()
{
    register struct macroposition *mpptr;

    do
    {
        mpptr = &macrostack[maclevel - 1];
        if (mpptr->inparam)
        {
            lineptr = ++mpptr->paramspot;
            mpptr->inparam = FALSE;
        }
        else
        {
            ch = *++lineptr;    /* gch1() would mess up next param == EOL-1 */
            if (ch != 0)
            {
                mpptr->paramspot = lineptr;
                lineptr = mpptr->paramlist[ch - 1];
                mpptr->inparam = TRUE;
            }
            else
            {
                lineptr = mpptr->maclineptr;
                if (mpptr->nparam != 0)
                {
                    register char **paramlist;

#ifdef TS
                    ts_s_macparam_tot -= sizeof *paramlist * mpptr->nparam;
#endif
                    paramlist = mpptr->paramlist;
                    do
                    {
#ifdef TS
                        ts_s_macparam_string_alloced_tot -= strchr(*paramlist, EOL) - *paramlist + 1;
#endif
                        ourfree(*paramlist++);
                    } while (--mpptr->nparam != 0);
                    ourfree(mpptr->paramlist);
                }
                --maclevel;
            }
        }
    } while ((ch = *lineptr) == EOL && maclevel != 0);
}

void predefine()
{
    definestring("__BCC__ 1");
    definestring("__LINE__ 0123456789");    /* MAX__LINE__ digits */
    findlorg("__LINE__")->storage = DEF_LINE;
}

char *savedlineptr()
{
    return macrostack[0].maclineptr;
}

void skipcomment()
{
    /* Skip current char, then everything up to '*' '/' or eof. */

    gch1();
    do
    {
        while (TRUE)
        {
            {
                register char *reglineptr;

                reglineptr = lineptr;
                symofchar['*'] = SPECIALCHAR;
                while (SYMOFCHAR(*reglineptr) != SPECIALCHAR)
                {
                    ++reglineptr;
                }
                symofchar['*'] = STAR;
                lineptr = reglineptr;
                if (*reglineptr == '*')
                {
                    break;
                }
                ch = *reglineptr;
            }
            specialchar();
            if (ch == EOL)
            {
                skipeol();
                if (eofile)
                {
                    break;
                }
            }
            else if (ch != '*')
            {
                gch1();
            }
        }
        gch1();
        if (eofile)
        {
            eofin("comment");
            return;
        }
    } while (ch != '/');
    gch1();
}

/* skipline() - skip rest of line */
void skipline()
{
    while (TRUE)
    {
        blanks();
        if (ch == EOL)
        {
            return;
        }
        if (ch == '\\')
        {
            gch1();
            if (ch == EOL)
            {    /* XXX - I think blanks() eats \EOL */
                return;
            }
            gch1();        /* XXX - escape() better? */
        }
        else if (ch == '"' || ch == '\'')
        {
            stringorcharconst();
            charptr = constant.value.s;
        }
        else
        {
            gch1();
        }
    }
}

/* undef() - process #undef */
static void undef()
{
    struct symstruct *symptr;

    if (blanksident() && (symptr = findlorg(gsname)) != NULL && symptr->flags == DEFINITION)
    {
        delsym(symptr);
    }
}

void undefinestring(char *str)
{
    defineorundefinestring(str, FALSE);
}