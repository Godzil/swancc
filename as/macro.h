/* macro.h - global variables for macro expansion for assembler */

extern bool_t macflag;		/* inside macro flag */
extern bool_t macload;		/* loading macro flag */
extern unsigned macnum;		/* macro call counter */

extern unsigned char maclevel;	/* nesting level */
extern struct schain_s *macpar;	/* parameter save buffer */
extern struct schain_s *macptop;	/* top of param buffer (+1) */
extern struct macro_s *macstak;	/* stack ptr */
