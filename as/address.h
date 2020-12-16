/* address.h - global variables involving addresses for assembler */

extern struct address_s lastexp;/* last expression parsed */

extern union
{
    char fcbuf[LINLEN - 6];	/* buffer for fcb and fcc data */
				/* data is absolute in 1 char pieces */
				/* limited by FCC\t"" etc on line */
    struct address_s fdbuf[(LINLEN - 4) / 2];
				/* buffer for fdb data */
				/* data can be of any 2-byte adr type */
				/* limited by FDB\t and commas on line */
#if SIZEOF_OFFSET_T > 2
    struct address_s fqbuf[(LINLEN - 4) / 4];
				/* buffer for fqb data */
				/* data can be of any 4-byte adr type */
				/* limited by FQB\t and commas on line */
#endif
}
    databuf;

extern bool_t fcflag;
extern bool_t fdflag;
#if SIZEOF_OFFSET_T > 2
extern bool_t fqflag;
#endif

extern struct address_s immadr;
extern smallcount_t immcount;
