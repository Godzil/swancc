/*
 * This file based on printf.c from 'Dlibs' on the atari ST  (RdeBath)
 *
 * 19-OCT-88: Dale Schumacher
 * > John Stanley has again been a great help in debugging, particularly
 * > with the printf/scanf functions which are his creation.  
 *
 *    Dale Schumacher                         399 Beacon Ave.
 *    (alias: Dalnefre')                      St. Paul, MN  55104
 *    dal@syntel.UUCP                         United States of America
 *  "It's not reality that's important, but how you perceive things."
 *
 */

/* Altered to use stdarg, made the core function vfprintf.
 * Hooked into the stdio package using 'inside information'
 * Altered sizeof() assumptions, now assumes all integers except chars
 * will be either
 *  sizeof(xxx) == sizeof(long) or sizeof(xxx) == sizeof(short)
 *
 * -RDB
 */

#include <sys/types.h>
#include <fcntl.h>
#ifdef __STDC__
#include <stdarg.h>
#define va_strt      va_start
#else
#include <varargs.h>
#define va_strt(p,i) va_start(p)
#endif

#include "stdio.h"

#ifdef L_printf

#ifdef __STDC__
int printf(const char * fmt, ...)
#else
int printf(fmt, va_alist)
__const char *fmt;
va_dcl
#endif
{
  va_list ptr;
  int rv;
  va_strt(ptr, fmt);
  rv = vfprintf(stdout,fmt,ptr);
  va_end(ptr);
  return rv;
}
#endif

#ifdef L_sprintf
#ifdef __STDC__
int sprintf(char * sp, const char * fmt, ...)
#else
int sprintf(sp, fmt, va_alist)
char * sp;
__const char *fmt;
va_dcl
#endif
{
static FILE  string[1] =
{
   {0, 0, (char*)(unsigned) -1, 0, (char*) (unsigned) -1, -1,
    _IOFBF | __MODE_WRITE}
};

  va_list ptr;
  int rv;
  va_strt(ptr, fmt);
  string->bufpos = sp;
  rv = vfprintf(string,fmt,ptr);
  va_end(ptr);
  *(string->bufpos) = 0;
  return rv;
}
#endif

#ifdef L_fprintf
#ifdef __STDC__
int fprintf(FILE * fp, const char * fmt, ...)
#else
int fprintf(fp, fmt, va_alist)
FILE * fp;
__const char *fmt;
va_dcl
#endif
{
  va_list ptr;
  int rv;
  va_strt(ptr, fmt);
  rv = vfprintf(fp,fmt,ptr);
  va_end(ptr);
  return rv;
}
#endif

#ifdef L_vprintf
int vprintf(fmt, ap)
__const char *fmt;
va_list ap;
{
  return vfprintf(stdout,fmt,ap);
}
#endif

#ifdef L_vsprintf
int vsprintf(sp, fmt, ap)
char * sp;
__const char *fmt;
va_list ap;
{
static FILE  string[1] =
{
   {0, 0, (char*)(unsigned) -1, 0, (char*) (unsigned) -1, -1,
    _IOFBF | __MODE_WRITE}
};

  int rv;
  string->bufpos = sp;
  rv = vfprintf(string,fmt,ap);
  *(string->bufpos) = 0;
  return rv;
}
#endif

#ifdef L_vfprintf

#ifdef FLOATS
int (*__fp_print)() = 0;
#endif

static int
prtfld(op, buf, ljustf, sign, pad, width, preci, buffer_mode)
register FILE *op;
register unsigned char *buf;
int   ljustf;
register char sign;
char  pad;
register int width;
int   preci;
int   buffer_mode;
/*
 * Output the given field in the manner specified by the arguments. Return
 * the number of characters output.
 */
{
   register int cnt = 0, len;
   register unsigned char ch;

   len = strlen(buf);

   if (*buf == '-')
      sign = *buf++;
   else if (sign)
      len++;

   if ((preci != -1) && (len > preci))	/* limit max data width */
      len = preci;

   if (width < len)		/* flexible field width or width overflow */
      width = len;

   /*
    * at this point: width = total field width len   = actual data width
    * (including possible sign character)
    */
   cnt = width;
   width -= len;

   while (width || len)
   {
      if (!ljustf && width)	/* left padding */
      {
	 if (len && sign && (pad == '0'))
	    goto showsign;
	 ch = pad;
	 --width;
      }
      else if (len)
      {
	 if (sign)
	 {
	  showsign:ch = sign;	/* sign */
	    sign = '\0';
	 }
	 else
	    ch = *buf++;	/* main field */
	 --len;
      }
      else
      {
	 ch = pad;		/* right padding */
	 --width;
      }
      putc(ch, op);
      if( ch == '\n' && buffer_mode == _IOLBF ) fflush(op);
   }

   return (cnt);
}

int
vfprintf(op, fmt, ap)
FILE *op;
register __const char *fmt;
register va_list ap;
{
   register int i, cnt = 0, ljustf, lval;
   int   preci, dpoint, width;
   char  pad, sign, radix, hash;
   register char *ptmp;
   char  tmp[64], *ltostr(), *ultostr();
   int buffer_mode;

   /* This speeds things up a bit for unbuffered */
   buffer_mode = (op->mode&__MODE_BUF);
   op->mode &= (~__MODE_BUF);

   while (*fmt)
   {
      if (*fmt == '%')
      {
         if( buffer_mode == _IONBF ) fflush(op);
	 ljustf = 0;		/* left justify flag */
	 sign = '\0';		/* sign char & status */
	 pad = ' ';		/* justification padding char */
	 width = -1;		/* min field width */
	 dpoint = 0;		/* found decimal point */
	 preci = -1;		/* max data width */
	 radix = 10;		/* number base */
	 ptmp = tmp;		/* pointer to area to print */
	 hash = 0;
	 lval = (sizeof(int)==sizeof(long));	/* long value flaged */
       fmtnxt:
	 i = 0;
	 for(;;)
	 {
	    ++fmt;
	    if(*fmt < '0' || *fmt > '9' ) break;
	    i = (i * 10) + (*fmt - '0');
	    if (dpoint)
	       preci = i;
	    else if (!i && (pad == ' '))
	    {
	       pad = '0';
	       goto fmtnxt;
	    }
	    else
	       width = i;
	 }

	 switch (*fmt)
	 {
	 case '\0':		/* early EOS */
	    --fmt;
	    goto charout;

	 case '-':		/* left justification */
	    ljustf = 1;
	    goto fmtnxt;

	 case ' ':
	 case '+':		/* leading sign flag */
	    sign = *fmt;
	    goto fmtnxt;

	 case '*':		/* parameter width value */
	    i = va_arg(ap, int);
	    if (dpoint)
	       preci = i;
	    else
	       width = i;
	    goto fmtnxt;

	 case '.':		/* secondary width field */
	    dpoint = 1;
	    goto fmtnxt;

	 case 'l':		/* long data */
	    lval = 1;
	    goto fmtnxt;

	 case 'h':		/* short data */
	    lval = 0;
	    goto fmtnxt;

	 case 'd':		/* Signed decimal */
	 case 'i':
	    ptmp = ltostr((long) ((lval)
			 ? va_arg(ap, long)
			 : va_arg(ap, short)),
		 10);
	    goto printit;

	 case 'b':		/* Unsigned binary */
	    radix = 2;
	    goto usproc;

	 case 'o':		/* Unsigned octal */
	    radix = 8;
	    goto usproc;

	 case 'p':		/* Pointer */
	    lval = (sizeof(char*) == sizeof(long));
	    pad = '0';
	    width = 6;
	    preci = 8;
	    /* fall thru */

	 case 'x':		/* Unsigned hexadecimal */
	 case 'X':
	    radix = 16;
	    /* fall thru */

	 case 'u':		/* Unsigned decimal */
	  usproc:
	    ptmp = ultostr((unsigned long) ((lval)
				   ? va_arg(ap, unsigned long)
				   : va_arg(ap, unsigned short)),
		  radix);
	    if( hash && radix == 8 ) { width = strlen(ptmp)+1; pad='0'; }
	    goto printit;

	 case '#':
	    hash=1;
	    goto fmtnxt;

	 case 'c':		/* Character */
	    ptmp[0] = va_arg(ap, int);
	    ptmp[1] = '\0';
	    goto nopad;

	 case 's':		/* String */
	    ptmp = va_arg(ap, char*);
	  nopad:
	    sign = '\0';
	    pad = ' ';
	  printit:
	    cnt += prtfld(op, ptmp, ljustf,
			   sign, pad, width, preci, buffer_mode);
	    break;

#if FLOATS
	 case 'e':		/* float */
	 case 'f':
	 case 'g':
	 case 'E':
	 case 'G':
	    if ( __fp_print )
	    {
	       (*__fp_print)(&va_arg(ap, double), *fmt, preci, ptmp);
	       preci = -1;
	       goto printit;
	    }
	    /* FALLTHROUGH if no floating printf available */
#endif

	 default:		/* unknown character */
	    goto charout;
	 }
      }
      else
      {
       charout:
	 putc(*fmt, op);	/* normal char out */
	 ++cnt;
         if( *fmt == '\n' && buffer_mode == _IOLBF ) fflush(op);
      }
      ++fmt;
   }
   op->mode |= buffer_mode;
   if( buffer_mode == _IONBF ) fflush(op);
   if( buffer_mode == _IOLBF ) op->bufwrite = op->bufstart;
   return (cnt);
}
#endif
