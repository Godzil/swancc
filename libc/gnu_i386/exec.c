
#include <errno.h>
#include <sys/stat.h>

extern char ** environ;

#ifdef L_execl
int
execl(fname, arg0)
char * fname, *arg0;
{
   return execve(fname, &arg0, environ);
}
#endif

#ifdef L_execv
int
execv(fname, argv)
char * fname, **argv;
{
   return execve(fname, argv, environ);
}
#endif

#ifdef L_execle
int
execle(fname, arg0)
char *fname, *arg0;
{
   char ** envp = &arg0;
   while(*envp) envp++;
   return execve(fname, &arg0, envp+1);
}
#endif

#ifdef L_execve
int
execve(fname, argv, envp)
char * fname;
char ** argv;
char ** envp;
{
	char **p;
	int argv_len=0, argv_count=0;
	int envp_len=0, envp_count=0;
	int stack_bytes;
	unsigned short * pip;
	char * pcp, * stk_ptr, *baseoff;
	int rv;

	/* How much space for argv */
	for(p=argv; p && *p && argv_len >= 0; p++)
	{
	   argv_count++; argv_len += strlen(*p)+1;
	}

	/* How much space for envp */
	for(p=envp; p && *p && envp_len >= 0; p++)
	{
	   envp_count++; envp_len += strlen(*p)+1;
	}

	/* tot it all up */
	stack_bytes = 2				/* argc */
	            + argv_count * 2 + 2	/* argv */
		    + argv_len
		    + envp_count * 2 + 2	/* envp */
		    + envp_len;

	/* Allocate it */
	if( argv_len < 0 || envp_len < 0 || stack_bytes <= 0
	 || (int)(stk_ptr = (char*)sbrk(stack_bytes)) == -1)
	{
	   errno = ENOMEM;
	   return -1;
	}

/* Sanity check 
	printf("Argv = (%d,%d), Envp=(%d,%d), stack=%d\n",
	        argv_count, argv_len, envp_count, envp_len, stack_bytes);
*/

	/* Now copy in the strings */
	pip=(unsigned short *) stk_ptr;
	pcp=stk_ptr+2*(1+argv_count+1+envp_count+1);

	/* baseoff = stk_ptr + stack_bytes; */
	baseoff = stk_ptr;
	*pip++ = argv_count;
	for(p=argv; p && *p; p++)
	{
	   int l;
	   *pip++ = pcp-baseoff;
	   l = strlen(*p)+1;
	   memcpy(pcp, *p, l);
	   pcp += l;
	}
	*pip++ = 0;

	for(p=envp; p && *p; p++)
	{
	   int l;
	   *pip++ = pcp-baseoff;
	   l = strlen(*p)+1;
	   memcpy(pcp, *p, l);
	   pcp += l;
	}
	*pip++ = 0;

	rv = __exec(fname, stk_ptr, stack_bytes);
	/* FIXME: This will probably have to interpret '#!' style exe's */
	sbrk(-stack_bytes);
	return rv;
}
#endif

#ifdef L_execlp
int
execlp(fname, arg0)
char * fname, *arg0;
{
   return execvp(fname, &arg0);
}
#endif

#ifdef L_execvp
int
execvp(fname, argv)
char * fname, **argv;
{
   char *pname = fname, *path;
   int besterr = ENOENT;
   int flen, plen;
   char * bp = sbrk(0);

   if( *fname != '/' && (path = getenv("PATH")) != 0 )
   {
      flen = strlen(fname)+2;

      for(;path;)
      {
         if( *path == ':' || *path == '\0' )
	 {
	    tryrun(fname, argv);
	    if( errno == EACCES ) besterr = EACCES;
	    if( *path ) path++; else break;
	 }
	 else
	 {
	    char * p = strchr(path, ':');
	    if(p) *p = '\0';
	    plen = strlen(path);
	    pname = sbrk(plen+flen);

	    strcpy(pname, path);
	    strcat(pname, "/");
	    strcat(pname, fname);

	    tryrun(pname, argv);
	    if( errno == EACCES ) besterr = EACCES;

	    brk(pname);
	    pname = fname;
	    if(p) *p++ = ':';
	    path=p;
	 }
      }
   }

   tryrun(pname, argv);
   brk(bp);
   if( errno == ENOENT || errno == 0 ) errno = besterr;
   return -1;
}

static int tryrun(pname, argv)
char * pname;
char ** argv;
{
static char *shprog[] = {"/bin/sh", "", 0};
   struct stat st;

   if( stat(pname, &st) < 0 ) return;
   if( !S_ISREG(st.st_mode) ) return;

#ifdef __AS386_16__
   __execvve(pname, (void*)0, argv, environ);
   if( errno == ENOEXEC )
   {
      shprog[1] = pname;
      __execvve(shprog[0], shprog, argv, environ);
   }
#else
   execve(pname, argv, environ);
   /* FIXME - running /bin/sh in 386 mode */
#endif
}

#ifdef __AS386_16__
static int
__execvve(fname, interp, argv, envp)
char * fname;
char ** interp;
char ** argv;
char ** envp;
{
	char **p;
	int argv_len=0, argv_count=0;
	int envp_len=0, envp_count=0;
	int stack_bytes;
	unsigned short * pip;
	char * pcp, * stk_ptr, *baseoff;
	int rv;

	/* How much space for argv */
	for(p=interp; p && *p && argv_len >= 0; p++)
	{
	   argv_count++; argv_len += strlen(*p)+1;
	}
	for(p=argv; p && *p && argv_len >= 0; p++)
	{
	   argv_count++; argv_len += strlen(*p)+1;
	}

	/* How much space for envp */
	for(p=envp; p && *p && envp_len >= 0; p++)
	{
	   envp_count++; envp_len += strlen(*p)+1;
	}

	/* tot it all up */
	stack_bytes = 2				/* argc */
	            + argv_count * 2 + 2	/* argv */
		    + argv_len
		    + envp_count * 2 + 2	/* envp */
		    + envp_len;

	/* Allocate it */
	if( argv_len < 0 || envp_len < 0 || stack_bytes <= 0
	 || (int)(stk_ptr = (char*)sbrk(stack_bytes)) == -1)
	{
	   errno = ENOMEM;
	   return -1;
	}

/* Sanity check 
	printf("Argv = (%d,%d), Envp=(%d,%d), stack=%d\n",
	        argv_count, argv_len, envp_count, envp_len, stack_bytes);
*/

	/* Now copy in the strings */
	pip=(unsigned short *) stk_ptr;
	pcp=stk_ptr+2*(1+argv_count+1+envp_count+1);

	/* baseoff = stk_ptr + stack_bytes; */
	baseoff = stk_ptr;
	*pip++ = argv_count;
	for(p=interp; p && *p; p++)
	{
	   int l;
	   *pip++ = pcp-baseoff;
	   l = strlen(*p)+1;
	   memcpy(pcp, *p, l);
	   pcp += l;
	}
	for(p=argv; p && *p; p++)
	{
	   int l;
	   *pip++ = pcp-baseoff;
	   l = strlen(*p)+1;
	   memcpy(pcp, *p, l);
	   pcp += l;
	}
	*pip++ = 0;

	for(p=envp; p && *p; p++)
	{
	   int l;
	   *pip++ = pcp-baseoff;
	   l = strlen(*p)+1;
	   memcpy(pcp, *p, l);
	   pcp += l;
	}
	*pip++ = 0;

	rv = __exec(fname, stk_ptr, stack_bytes);
	/* FIXME: This will probably have to interpret '#!' style exe's */
	sbrk(-stack_bytes);
	return rv;
}
#endif
#endif
