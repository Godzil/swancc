/*
 *	ctype.h		Character classification and conversion
 */

#ifndef __CTYPE_H
#define __CTYPE_H

extern	unsigned char	__ctype[];

#define	__CT_d	0x01		/* numeric digit */
#define	__CT_u	0x02		/* upper case */
#define	__CT_l	0x04		/* lower case */
#define	__CT_c	0x08		/* control character */
#define	__CT_s	0x10		/* whitespace */
#define	__CT_p	0x20		/* punctuation */
#define	__CT_x	0x40		/* hexadecimal */

#define	toupper(c)	(islower(c) ? (c)^0x20 : (c))
#define	tolower(c)	(isupper(c) ? (c)^0x20 : (c))
#define	_toupper(c)	((c)^0x20)
#define	_tolower(c)	((c)^0x20)
#define	toascii(c)	((c)&0x7F)

/* Note the '!!' is a cast to 'bool' and even BCC deletes it in an if()  */
#define	isalnum(c)	(!!(__ctype[(int) c]&(__CT_u|__CT_l|__CT_d)))
#define	isalpha(c)	(!!(__ctype[(int) c]&(__CT_u|__CT_l)))
#define	isascii(c)	(!((c)&~0x7F))
#define	iscntrl(c)	(!!(__ctype[(int) c]&__CT_c))
#define	isdigit(c)	(!!(__ctype[(int) c]&__CT_d))
#define	isgraph(c)	(!(__ctype[(int) c]&(__CT_c|__CT_s)))
#define	islower(c)	(!!(__ctype[(int) c]&__CT_l))
#define	isprint(c)	(!(__ctype[(int) c]&__CT_c))
#define	ispunct(c)	(!!(__ctype[(int) c]&__CT_p))
#define	isspace(c)	(!!(__ctype[(int) c]&__CT_s))
#define	isupper(c)	(!!(__ctype[(int) c]&__CT_u))
#define	isxdigit(c)	(!!(__ctype[(int) c]&__CT_x))

#endif /* __CTYPE_H */
