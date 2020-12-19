/* state.c - statement routines for swancc
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
#include <swancc/state.h>
#include <swancc/align.h>
#include <swancc/condcode.h>
#include <swancc/gencode.h>
#include <swancc/input.h>        /* just for ch and eof for label check */
#include <swancc/label.h>
#include <swancc/os.h>            /* just for EOL */
#include <swancc/parser.h>
#include <swancc/reg.h>
#include <swancc/scan.h>
#include <swancc/sizes.h>
#include <swancc/table.h>
#include <swancc/type.h>
#include <swancc/output.h>
#include <swancc/preserve.h>
#include <swancc/declare.h>
#include <swancc/function.h>
#include <swancc/express.h>
#include <swancc/loadexp.h>
#include <swancc/codefrag.h>
#include <swancc/glogcode.h>
#include <swancc/preproc.h>

#define COLONCHAR ':'        /* for label */
#define GROWCASES    64    /* extra cases for growth of switch struct */
#define INITIALCASES    16    /* initial cases in switch structure */

struct loopstruct
{
    label_no breaklab;        /* destination for break */
    label_no contlab;        /* destination for continue */
    struct nodestruct *etmark;    /* expression tree built during loop */
    struct symstruct *exprmark;    /* expression symbols built during loop */
    struct symstruct *locmark;    /* local variables built during loop */
    struct loopstruct *prevloop;   /* previous active for, switch or while */
    offset_T spmark;        /* stack value for continue and break */
};

struct switchstruct
{
    struct casestruct *caseptr;    /* current spot in caselist */
    struct casestruct *casetop;    /* last in caselist + 1 */
    bool_t charselector;    /* tells if case selector is char */
    label_no dfaultlab;        /* destination for default case (0 if none) */
    struct switchstruct *prevswitch;    /* previous active switch */
    struct casestruct
    {
        value_t casevalue;    /* value giving this case */
        label_no caselabel;    /* corresponding label */
    } caselist[INITIALCASES];    /* perhaps larger */
};

static struct loopstruct *loopnow;    /* currently active for/switch/while */
/* depends on NULL init */
static bool_t returnflag;    /* set if last effective statement */
/* was a return */
static label_no swstacklab;    /* label giving stack for switch statement */

static void addloop(struct loopstruct *newloop);

static void badloop(void);

static void deleteloop(void);

static void evalexpression(struct nodestruct *exp);

static void exprstatement(void);

static bool_t isforever(struct nodestruct *exp);

static void sort(struct casestruct *caselist, int count);

static void dobreak(void);

static void docase(void);

static void docont(void);

static void dodefault(void);

static void dodowhile(void);

static void dofor(void);

static void dogoto(void);

static void doif(void);

static void doreturn(void);

static void doswitch(void);

static void dowhile(void);

static void jumptocases(void);

static void statement(void);

static void doasm(void);

/* --- utility routines --- */

static void addloop(struct loopstruct *newloop)
{
    newloop->breaklab = getlabel();
    newloop->contlab = getlabel();
    newloop->etmark = etptr;
    newloop->exprmark = exprptr;
    newloop->locmark = locptr;
    newloop->prevloop = loopnow;
    newloop->spmark = sp;
    loopnow = newloop;
}

static void badloop()
{
    error(" no active fors, switches or whiles");
}

static void deleteloop()
{
    etptr = loopnow->etmark;
    exprptr = loopnow->exprmark;
    locptr = loopnow->locmark;
    loopnow = loopnow->prevloop;
}

static void evalexpression(struct nodestruct *exp)
{
    offset_T spmark;

    spmark = sp;
    makeleaf(exp);
    modstk(spmark);
}

static void exprstatement()
{
    struct nodestruct *etmark;
    struct symstruct *exprmark;

    etmark = etptr;
    exprmark = exprptr;
    evalexpression(expression());
    etptr = etmark;
    exprptr = exprmark;
}

static bool_t isforever(struct nodestruct *exp)
{
    return exp == NULL ||
           (exp->tag == LEAF && exp->left.symptr->storage == CONSTANT && exp->left.symptr->offset.offv != 0);
}

/* shell sort */
static void sort(struct casestruct *caselist, int count)
{
    int gap;
    int i;
    int j;
    struct casestruct swaptemp;

    gap = 1;
    do
    {
        gap = 3 * gap + 1;
    } while (gap <= count);
    while (gap != 1)
    {
        gap /= 3;
        for (j = gap ; j < count ; ++j)
        {
            for (i = j - gap ; i >= 0 && caselist[i].casevalue > caselist[i + gap].casevalue ; i -= gap)
            {
                swaptemp = caselist[i];
                caselist[i] = caselist[i + gap];
                caselist[i + gap] = swaptemp;
            }
        }
    }
}

/* --- syntax routines --- */

/*
 * compound-statement:
 *    "{" <declaration-list> <statement-list> "}"
 */
void compound()        /* have just seen "{" */
{
    struct symstruct *locmark;
    store_t regmark;
#ifdef FRAMEPOINTER
    offset_T framepmark;
    offset_T softspmark;
#else
    /*
     * softsp == sp here unless level == ARGLEVEL so mark is unnec */
     * this is also true if funcsaveregsize != 0 but the tests are too messy
     */
#endif
    offset_T spmark;

    locmark = locptr;
    regmark = reguse;
#ifdef FRAMEPOINTER
    softspmark = softsp;
    framepmark = framep;
#endif
    spmark = sp;
    newlevel();
    expect_statement++;
    decllist();
    softsp &= alignmask;
    if (sym != RBRACE)
    {        /* no need for locals if empty compound */
        reslocals();
    }
    returnflag = FALSE;
    while (sym != RBRACE && sym != EOFSYM)
    {
        statement();
    }
    expect_statement--;
    oldlevel();
    if (!returnflag)
    {
        if (level != ARGLEVEL)
        {
            if (switchnow == NULL)
            {
#ifdef FRAMEPOINTER
                if (framep != 0)
                {
                    /* some args or locals, maybe at lower levels */
                    modstk(softspmark);
                    if (framep != framepmark)
                    {
                        /* frame was just set up */
                        popframe();
                    }
                }
#else
                modstk(spmark);
#endif
            }
        }
        else
        {
            ret();
        }
    }
#ifdef FRAMEPOINTER
    framep = framepmark;
    sp = spmark;
    softsp = softspmark;
#else
    softsp = sp = spmark;
#endif
    reguse = regmark;
    locptr = locmark;
    rbrace();
}

static void doasm()
{
    if (sym == LPAREN)
    {
        nextsym();
    }
    if (sym != STRINGCONST)
    {
        error("string const expected");
    }
    else
    {
        outnstr("!BCC_ASM");
        for (;;)
        {
            constant.value.s[charptr - constant.value.s] = '\0';
            outbyte('\t');
            outnstr(constant.value.s);
            /* XXX: Need to investigate: wasting memory?
             *
             * charptr = constant.value.s;
             */

            nextsym();
            if (sym == COMMA)
            {
                nextsym();
            }
            if (sym != STRINGCONST)
            {
                break;
            }
        }
        outnstr("!BCC_ENDASM");
        if (sym == RPAREN)
        {
            nextsym();
        }
        semicolon();
    }
}

static void dobreak()
{
    offset_T spmark;

    if (loopnow == NULL)
    {
        badloop();
    }
    else
    {
        if (switchnow == NULL)
        {
            spmark = sp;
            modstk(loopnow->spmark);
            sp = spmark;
        }
        jump(loopnow->breaklab);
    }
}

static void docase()
{
    value_t caseval;

    caseval = constexpression() & intmaskto;    /* FIXME: warn overflow */
    if (caseval > maxintto)
    {
        caseval -= (maxuintto + 1);
    }
    colon();
    if (switchnow == NULL)
    {
        error("case not in switch");
    }
    else
    {
        if (switchnow->charselector && (caseval < 0 || caseval > 255))
        {
            error("%wcase cannot be reached with char switch");
        }
        else
        {
            if (switchnow->caseptr >= switchnow->casetop)
            {
                int ncases;

                ncases = (/* ptrdiff_t */ int)(switchnow->caseptr - &switchnow->caselist[0]);
                switchnow = realloc(switchnow, (/* size_t */ uint32_t)((char *)switchnow->caseptr - (char *)switchnow) +
                                               GROWCASES * sizeof(struct casestruct));
#ifdef TS
                ++ts_n_case_realloc;
                ts_s_case += GROWCASES * sizeof (struct casestruct);
                ts_s_case_tot += GROWCASES * sizeof (struct casestruct);
#endif
                if (switchnow == NULL)
                {
                    outofmemoryerror("");
                }
                switchnow->caseptr = &switchnow->caselist[ncases];
                switchnow->casetop = switchnow->caseptr + GROWCASES;
            }
            switchnow->caseptr->casevalue = caseval;
            deflabel(switchnow->caseptr->caselabel = getlabel());
            ++switchnow->caseptr;
        }
    }
}

static void docont()
{
    struct loopstruct *contloop;
    offset_T spmark;
    struct switchstruct *switchthen;

    for (contloop = loopnow, switchthen = switchnow ;; contloop = contloop->prevloop, switchthen = switchthen->prevswitch)
    {
        if (contloop == NULL)
        {
            badloop();
            return;
        }
        if (contloop->contlab != 0)
        {
            break;
        }
    }
    if (switchnow == NULL)
    {
        spmark = sp;
        modstk(contloop->spmark);
        sp = spmark;
    }
    else if (switchthen == NULL)
    {
        outaddsp();
        outshex(contloop->spmark);
        outminus();
        outswstacklab();
#ifdef MC6809
        outcregname(LOCAL);
#endif
#ifdef I80386
        if (i386_32)
        {
            bumplc2();
        }
#endif
        outnl();
    }
    jump(contloop->contlab);
}

static void dodefault()
{
    colon();
    if (switchnow == NULL)
    {
        error("default not in switch");
    }
    else if (switchnow->dfaultlab != 0)
    {
        error("multiple defaults");
    }
    else
    {
        deflabel(switchnow->dfaultlab = getlabel());
    }
}

static void dodowhile()
{
    struct loopstruct dwhileloop;
    label_no dolab;

    addloop(&dwhileloop);
    deflabel(dolab = getlabel());
    statement();
    if (sym == WHILESYM)
    {
        nextsym();
    }
    else
    {
        error("missing while at end of do-while");
    }
    deflabel(dwhileloop.contlab);
    lparen();
    jumptrue(expression(), dolab);
    rparen();
    semicolon();
    deflabel(dwhileloop.breaklab);
    deleteloop();
}

static void dofor()
{
    struct loopstruct forloop;
    label_no forstatlab;
    label_no fortestlab = 0; /* for -Wall */
    struct nodestruct *testexp;
    struct nodestruct *loopexp;

    lparen();
    if (sym != SEMICOLON)
    {
        exprstatement();
    }
    semicolon();
    addloop(&forloop);
    if (sym == SEMICOLON)
    {
        testexp = NULL;
    }
    else
    {
        testexp = expression();
    }    /* remember test expression */
    semicolon();
    if (sym == RPAREN)
        loopexp = NULL;
    else
        loopexp = expression();    /* remember loop expression */
    rparen();
    if (!isforever(testexp))
        jump(fortestlab = getlabel());    /* test at bottom */
    deflabel(forstatlab = getlabel());    /* back here if test succeeds */
    statement();
    deflabel(forloop.contlab);
    if (loopexp != NULL)
        evalexpression(loopexp);
    if (isforever(testexp))
        jump(forstatlab);
    else
    {
        deflabel(fortestlab);    /* test label */
        jumptrue(testexp, forstatlab);
    }
    deflabel(forloop.breaklab);
    deleteloop();
}

static void dogoto()
{
    struct symstruct *symptr;

    if (sym == IDENT || sym == TYPEDEFNAME)
    {
        symptr = namedlabel();
        /*
            if (symptr->indcount == 4)
                modstk(
            else
        */
        adjsp(symptr->offset.offlabel);
        jump(symptr->offset.offlabel);
        nextsym();
    }
    else
    {
        error("need identifier");
    }
}

static void doif()
{
    struct nodestruct *etmark;
    label_no elselab;
    label_no exitlab;
    struct symstruct *exprmark;

    lparen();
    etmark = etptr;
    exprmark = exprptr;
    jumpfalse(expression(), elselab = getlabel());
    etptr = etmark;
    exprptr = exprmark;
    rparen();
    statement();        /* true, do a statement */
    if (sym == ELSESYM)        /* "if .. else" statement */
    {
        nextsym();
        jump(exitlab = getlabel());    /* over "else" label and code */
        deflabel(elselab);
        statement();
        deflabel(exitlab);
    }
    else
    {
        deflabel(elselab);
    }
}

static void doreturn()
{
    offset_T spmark;

    spmark = sp;
    if (sym != SEMICOLON)
    {    /* returning expression */
        loadretexpression();
    }
    ret();            /* clean up stack and return */
    sp = spmark;        /* restore stack for rest of function */
}

static void doswitch()
{
    struct switchstruct *sw;
    struct loopstruct switchloop;
    offset_T spmark = 0; /* for -Wall */
    label_no sdecidelab;

    sw = (struct switchstruct *)ourmalloc(sizeof *sw);
#ifdef TS
    ++ts_n_case;
    ts_s_case += sizeof *sw;
    ts_s_case_tot += sizeof *sw;
#endif
    lparen();
    sw->charselector = loadexpression(DREG, NULLTYPE)->scalar & CHAR;
    rparen();
    if (switchnow == NULL)
    {
        spmark = lowsp = sp;
    }
    addloop(&switchloop);
    sw->dfaultlab = switchloop.contlab = 0; /* kill to show this is a switch */
    sw->casetop = (sw->caseptr = sw->caselist) + INITIALCASES;
    sw->prevswitch = switchnow;
    jump(sdecidelab = getlabel());    /* to case decision label */
    if (switchnow == NULL)
    {
        swstacklab = gethighlabel();
    }
    switchnow = sw;
    statement();        /* cases */
    sw = switchnow;
    jump(switchloop.breaklab);    /* over case decision to break label */
    deflabel(sdecidelab);
    if (sw->prevswitch == NULL)
    {
        modstk(lowsp);
    }
    jumptocases();
    deflabel(switchloop.breaklab);
    if ((switchnow = sw->prevswitch) == NULL)
    {
        equlab(swstacklab, lowsp);
        clearswitchlabels();
        modstk(spmark);
    }
    deleteloop();
#ifdef TS
    ts_s_case_tot -= (char *) sw->casetop - (char *) sw;
#endif
    ourfree(sw);
}

static void dowhile()
{
    struct loopstruct whileloop;
    struct nodestruct *testexp;
    label_no wstatlab;

    lparen();
    addloop(&whileloop);
    testexp = expression();
    rparen();
    if (!isforever(testexp))
    {
        jump(whileloop.contlab);
    }    /* test at bottom */
    deflabel(wstatlab = getlabel());    /* back here if test succeeds */
    statement();
    deflabel(whileloop.contlab);
    jumptrue(testexp, wstatlab);
    deflabel(whileloop.breaklab);
    deleteloop();
}

static void jumptocases()
{
    value_t basevalue;
    struct casestruct *case1ptr;
    struct casestruct *caseptr;
    struct casestruct *casetop;
    value_t caseval;
    bool_t charselector;
    bool_t dfaultflag;
    label_no dfaultlab;
    label_no jtablelab;
    ccode_t lowcondition;
    store_t targreg;
    label_no zjtablelab;

    caseptr = switchnow->caselist;
    casetop = switchnow->caseptr;
    sort(caseptr, (/* ptrdiff_t */ int)(casetop - caseptr));
    basevalue = 0;
    if ((charselector = switchnow->charselector) != 0)
    {
        targreg = BREG;
        lowcondition = LO;
    }
    else
    {
        targreg = DREG;
        lowcondition = LT;
    }
    dfaultflag = TRUE;
    if ((dfaultlab = switchnow->dfaultlab) == 0)
    {
        dfaultflag = FALSE;
        dfaultlab = loopnow->breaklab;
    }
    --casetop;
    for (case1ptr = caseptr ; case1ptr < casetop ; ++case1ptr)
    {
        if (case1ptr->casevalue == (case1ptr + 1)->casevalue)
        {
            error("duplicate case in switch");
        }
    }
    while (caseptr <= casetop)
    {
        outsub();
        outimadj((offset_T)(caseptr->casevalue - basevalue), targreg);
        basevalue = caseptr->casevalue;
        for (case1ptr = caseptr ; case1ptr < casetop ; ++case1ptr)
        {
            if (case1ptr->casevalue < (case1ptr + 1)->casevalue - 10)
            {
                break;
            }
        }
        if (case1ptr < caseptr + 5)
        {
            lbranch(EQ, caseptr->caselabel);
            ++caseptr;
        }
        else
        {
            lbranch(lowcondition, dfaultlab);
            outcmp();
            outimadj((offset_T)(case1ptr->casevalue - basevalue), targreg);
            lbranch(HI, zjtablelab = getlabel());
            if (charselector)
            {
                ctoi();
            }
#ifdef MC6809
            else
            bumplc();    /* extra for CMPD */
#endif
            slconst((value_t)(ptypesize / 2), DREG);
            /* really log ptypesize */
            deflabel(jtablelab = casejump());
#ifdef I8088
            jtablelab = jtablelab;     /* not used, allocated for regress */
#endif
            for (caseval = caseptr->casevalue ; caseval <= case1ptr->casevalue ; ++caseval)
            {
#ifdef I8088
                if (ptypesize > 2)
                {
                    defdword();
                }
                else
#endif
                {
                    defword();
                }
                if (caseval != caseptr->casevalue)
                {
                    outlabel(dfaultlab);
                }
                else
                {
                    outlabel(caseptr->caselabel);
                    ++caseptr;
                }
#ifdef MC6809
                if (posindependent)
                {
                    outminus();
                    outlabel(jtablelab);
                }
#endif
                bumplc2();
#ifdef I8088
                if (ptypesize > 2)
                {
                    bumplc2();
                }
#endif
                outnl();
            }
            deflabel(zjtablelab);
        }
        lowcondition = LO;    /* 1st subtraction makes rest unsigned */
    }
    if (dfaultflag)
    {
        jump(dfaultlab);
    }
}

void outswoffset(offset_T offset)
{
#ifdef FRAMEPOINTER
    outoffset(offset - softsp - framep);
#else
    outoffset(offset - softsp - sp);
#endif
    outplus();
    outswstacklab();
#ifdef I8088
    bumplc();
#ifdef I80386
    if (i386_32)
    {
        bumplc2();
    }
#endif
#endif
}

void outswstacklab()
{
    outbyte(LOCALSTARTCHAR);
    outlabel(swstacklab);
}

/*
 * statement:
 *    <compound-statement>
 *    <if-statement>
 *    <while-statement>
 *    <do-statement>
 *    <for-statement>
 *    <switch-statement>
 *    <case-statement>
 *    <default-statement>
 *    <break-statement>
 *    <continue-statement>
 *    <return-statement>
 *    <goto-statement>
 *    <labelled-statement>
 *    <null-statement>
 *    <expression> ;
 */
static void statement()
{
#if 1
    struct symstruct *symptr;
#endif

    more:
    switch (sym)
    {
        case LBRACE:
            nextsym();
            compound();
            return;
        case IFSYM:
            nextsym();
            doif();
            break;
        case ELSESYM:
            error("unexpected else");
            nextsym();
            break;
        case WHILESYM:
            nextsym();
            dowhile();
            break;
        case FORSYM:
            nextsym();
            dofor();
            break;
        case RETURNSYM:
            nextsym();
            doreturn();
            semicolon();
            returnflag = TRUE;
            return;
        case SWITCHSYM:
            nextsym();
            doswitch();
            break;
        case BREAKSYM:
            nextsym();
            dobreak();
            semicolon();
            break;
        case CASESYM:
            nextsym();
            docase();
            goto more;
        case DEFAULTSYM:
            nextsym();
            dodefault();
            goto more;
        case DOSYM:
            nextsym();
            dodowhile();
            break;
        case CONTSYM:
            nextsym();
            docont();
            semicolon();
            break;
        case GOTOSYM:
            nextsym();
            dogoto();
            semicolon();
            break;
        case SEMICOLON:
            nextsym();
            return;
        case ASMSYM:
            nextsym();
            doasm();
            break;
        case IDENT:
        case TYPEDEFNAME:
            blanks();        /* cannot afford nextsym() */
            while (ch == EOL && !eofile)
            {
                /* this now fails only on #controls and macros giving ':' */
                skipeol();
                blanks();
            }
            if (ch == COLONCHAR)
            {
#if 0
                struct symstruct *symptr;
#endif
                symptr = namedlabel();
                if (symptr->indcount != 2)
                {
                    error("redefined label");
                }
                else
                {
                    deflabel(symptr->offset.offlabel);
                    if (switchnow != NULL)
                    {
                        symptr->indcount = 3;
                    }
                    else
                    {
                        symptr->indcount = 4;
                        outbyte(LOCALSTARTCHAR);
                        outlabel(symptr->offset.offlabel);
                        outop0str("\t=\t");
                        outshex(sp);
                        outnl();
                    }
                }
                nextsym();
                nextsym();
                goto more;
            }
            /* else fall into default */
        default:
            exprstatement();
            semicolon();
            break;
    }
    returnflag = FALSE;
}
