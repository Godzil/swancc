/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

extern char ** environ;

char *
getenv(name)
char * name;
{
   register int l;
   register char ** ep = environ;
   l = strlen(name);

   if( ep == 0 || l == 0 ) return 0;

   while(*ep)
   {
      if( **ep == *name && memcmp(name, *ep, l) == 0 && (*ep)[l] == '=' )
         return *ep+l+1;
      ep++;
   }
   return 0;
}

