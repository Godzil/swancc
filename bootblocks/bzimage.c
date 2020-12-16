/*
 * Load and execute a bzImage file from the device the 'open_file' and
 * friends use.
 */

#define __MINI_MALLOC__
#include "monitor.h"

int auto_flag = 1;

static char * initrd_name = 0;	/* Name of init_ramdisk to load */
static long initrd_start = 0;
static long initrd_length = 0;
static int vga_mode = -1;	/* SVGA_MODE = normal */

static int  is_zimage = 0;
static char * image_name = 0;
static int  image_length;	/* Length of image in sectors */
static long image_size;		/* Length of image file in bytes */

static char * read_cmdfile();
static char * input_cmd();

#define ZIMAGE_LOAD_SEG	 0x10000 /* Segment that zImage data is loaded */
#define COMMAND_LINE_POS 0x4000 /* Offset in segment 0x9000 of command line */

static char * linux_command_line = 0;
int load_crc = 0;

static char buffer[1024];

cmd_bzimage(ptr)
register char * ptr;
{
   char * image;
   int ch;

   if( (auto_flag=(ptr==0)) ) ptr="";

   while(*ptr == ' ') ptr++;
   image = ptr;
   while(*ptr != ' ' && *ptr) ptr++;
   ch = *ptr;
   *ptr = '\0';
   if( ch ) ptr++;
   while(*ptr == '\n' || *ptr == ' ') ptr++;

   if( *ptr == '\0' ) ptr = 0;
   if( *image )
      bzimage(image, ptr);
   else
      bzimage("vmlinuz", ptr);
}

bzimage(fname, command_line)
char * fname;
char * command_line;
{
   long len;
   char * ptr;
   int low_sects;
   unsigned int address;
   
   initrd_name = 0;
   initrd_start = initrd_length = 0;
   vga_mode = -1;
   is_zimage = 0;
   image_name = strdup(fname);

   /* Ok, deal with the command line */
   if( build_linuxarg(fname, command_line) < 0 )
      return -1;

   if( open_file(image_name) < 0 )
   {
      if( auto_flag == 0 )
         printf("Cannot find file %s\n", image_name);
      return -1;
   }

   printf("Loading %s\n", image_name);

   if( read_block(buffer) < 0 || check_magics(image_name, buffer) < 0 )
   {
      printf("Cannot execute file %s\n", image_name);
      return -1;
   }

#ifndef __ELKS__
   if( boot_mem_top < 0x9500 )
   {
      printf("There must be 640k of boot memory to load Linux\n");
      return -1;
   }

   /* Guestimate how much the uncompressed kernel will use.
    * I expect we could lookup the size in the gzip header but
    * this is probably close enough (3*the size of the bzimage)
    */
   len = (image_size=file_length()) * 3 / 1024;
   if( main_mem_top < len )
   {
      printf("This kernel needs at least %d.%dM of main memory\n",
              (int)(len/1024), (int)(len*10/1024%10));
      return -1;
   }
   if( main_mem_top < 3072 )
      printf("RTFM warning: Linux needs at least 4MB of memory.\n");

   len = (len+1023)/1024+1;		/* Where to load the RD image (Mb) */
   if (len<6) len=6;			/* Default to 6Mb mark */
   initrd_start = len * 4096;		/* 256 bytes pages. */
#endif

   low_sects    = buffer[497] + 1; /* setup sects + boot sector */
   if (low_sects == 1) low_sects = 5;
   image_length = (file_length()+511)/512 - low_sects;
   address = 0x900;

#ifndef __ELKS__
#if ZIMAGE_LOAD_SEG == 0x10000
   if( is_zimage )
   {
      if( image_length > 0x7FF0/32 )
      {
         printf("This zImage file is too large, maximum is %dk bytes\n",
                 (0x7FF0/32 + low_sects)/2 );
         return -1;
      }
   }
#else
   if( is_zimage )
   {
      relocator(8);	/* Need space in low memory */

      if( image_length > (__get_cs()>>5) - ZIMAGE_LOAD_SEG/32 )
      {
         printf("This zImage file is too large, maximum is %dk bytes\n",
                 ((__get_cs()>>5) - ZIMAGE_LOAD_SEG/32 + low_sects)/2 );
         return -1;
      }
   }
#endif
#endif

   /* load the blocks */
   rewind_file();
#ifdef CALC_CRC
   reset_crc();
#endif
   for(len = file_length(); len>0; len-=1024)
   {
      int v;

      printf("%dk to go \r", (int)(len/1024)); fflush(stdout);

#ifndef NOCOMMAND
      v = (kbhit()&0x7F);
      if( v == 3 || v == 27 )
      {
	 printf("User interrupt!\n");
         getch();
	 return -1;
      }
#endif

      if( read_block(buffer) < 0 )
      {
         printf("Error loading %s\n", image_name);
	 return -1;
      }

#ifdef CALC_CRC
      if( len > 1024 ) addcrc(buffer, 1024); else addcrc(buffer, (int)len);
#endif
      for(v=0; v<1024; v+=512)
      {
         if( putsect(buffer+v, address) < 0 )
	    return -1;

	 address += 2;

         if( low_sects )
	 {
	    low_sects--;
	    if( low_sects == 0 )
	    {
#if ZIMAGE_LOAD_SEG != 0x10000
	       if( is_zimage ) address = ZIMAGE_LOAD_SEG/16;
	       else            
#endif
	                       address = 0x1000;
	    }
	 }
      }
   }
   close_file();

#ifdef CALC_CRC
   load_crc = display_crc("Loaded crc =");
#endif

#ifdef CALC_CRC
   if( check_crc() < 0 && !keep_going() ) return -1;
#endif

   printf("          \r"); fflush(stdout);

   if( initrd_name )
      if( load_initrd(address) < 0 )
         return -1;

#ifdef CALC_CRC
   if( check_crc() < 0 && !keep_going() ) return -1;
#endif

   if( x86 < 3 || x86_emu )
   {
      if( x86 < 3 )
	 printf("RTFM error: Linux-i386 needs a CPU >= 80386\n");
      else if( x86_emu )
	 printf("RTFM error: Linux-i386 cannot be run in an emulator.\n");
      if( !keep_going() ) return -1;
   }

   printf("linux ");
   if( linux_command_line )
      printf("%s", linux_command_line);
   printf("\n");
   fflush(stdout);

   if( a20_closed() ) open_a20();
   if( a20_closed() )
   {
      printf("Normal routine for opening A20 Gate failed, Trying PS/2 Bios\n");
      bios_open_a20();
   }
   if( a20_closed() )
   {
      printf("All routines for opening A20 Gate failed, if I can't open it\n");
      printf("then Linux probably can't either!\n");
      if( !keep_going() ) return -1;
   }

   __set_es(0x9000);

   /* Save pointer to command line */
   if( linux_command_line )
   {
      __doke_es(0x0020, 0xA33F);
      __doke_es(0x0022, COMMAND_LINE_POS);	
      __movedata(__get_ds(), (unsigned)linux_command_line+1, 0x9000, COMMAND_LINE_POS, strlen(linux_command_line));
      free(linux_command_line);
   }

   __set_es(0x9000);

#if ZIMAGE_LOAD_SEG != 0x10000
#if ZIMAGE_LOAD_SEG != 0x1000
   if( is_zimage )
   {
#if ZIMAGE_LOAD_SEG != 0x100
      /* Tell setup that we've loaded the kernel somewhere */
      __doke_es(0x20C, ZIMAGE_LOAD_SEG);
#else
      /* Tell setup it's a bzImage _even_ tho it's a _zImage_ because we have
       * actually loaded it where it's supposed to end up!
       */
      __poke_es(0x211, __peek_es(0x211)|1);

      __poke_es(0x210, 0xFF); /* Patch setup to deactivate safety switch */
#endif
   }
#endif
#endif

   /* Tell the kernel where ramdisk is */
   if( initrd_name )
   {
      __set_es(0x9000);

      __doke_es(0x218, (unsigned) initrd_start);
      __doke_es(0x21A, (unsigned)(initrd_start>>16));

      __doke_es(0x21C, (unsigned) initrd_length);
      __doke_es(0x21E, (unsigned)(initrd_length>>16));
   }


   if( !is_zimage || initrd_name )
      __poke_es(0x210, 0xFF); /* Patch setup to deactivate safety switch */

   /* Set SVGA_MODE if not 'normal' */
   if( vga_mode != -1 ) __doke_es(506, vga_mode);

   /* Default boot drive is auto-detected floppy */
   if( __peek_es(508) == 0 ) __poke_es(508, 0x200);

   printf("Starting ...\n");;

#if ZIMAGE_LOAD_SEG == 0x10000
   if( is_zimage )
      /* Copy 512k from high memory then start */
      start_zimage();
   else
#endif

   /* Finally do the deed */
   {
#asm
  ! Kill the floppy motor, needed in case the kernel has no floppy driver.
  mov	dx,#0x3f2
  xor	al, al
  outb

  ! Setup required registers and go ...
  mov	ax,#$9000
  mov	bx,#$4000-12	! Fix this to use boot_mem_top
  mov	es,ax
  mov	fs,ax
  mov	gs,ax
  mov	ds,ax
  mov	ss,ax
  mov	sp,bx
  jmpi	0,$9020		! Note SETUPSEG NOT INITSEG
#endasm
   }
}

check_magics(fname, image_buf)
register char * fname;
register char * image_buf;
{
   is_zimage = 0;

   /* Boot sector magic numbers */
   if( *(unsigned short*)(image_buf+510) != 0xAA55 ||
      memcmp(image_buf, "\270\300\007\216") != 0 )
   {
      printf("File %s is not a linux Image file\n", fname);
      return -1;
   }

   /* Setup start */
   if ( memcmp(image_buf+0x202, "HdrS", 4) == 0 &&
   /* Setup version */
       *(unsigned short*)(image_buf+0x206) >= 0x200 )
   {
      /* Code 32 start address for zImage */
      if( *(unsigned long*)(image_buf+0x214) == 0x1000 )
      {
	 printf("File %s is a zImage file\n", fname);
	 is_zimage = 1;
	 return 0;
      }
      else
      /* Code 32 start address bzImage */
      if( *(unsigned long*)(image_buf+0x214) == 0x100000 )
      {
	 printf("File %s is a bzImage file\n", fname);
	 return 0;
      }
   }

   is_zimage = 1;
   printf("File %s is an old Image file\n", fname);
#if ZIMAGE_LOAD_SEG == 0x10000
   return 0;
#else
   return -1;
#endif
}

#ifndef __ELKS__
putsect(put_buf, address)
char * put_buf;
unsigned int address;
{
   int rv, tc=3;
   /* In real mode memory, just put it directly */
   if( address < 0xA00 )
   {
      __movedata(__get_ds(), put_buf, address*16, 0, 512);
      return 0;
   }

retry:
   tc--;

   if( x86_test )
      return 0;	/* In an EMU we can't write to high mem but
                   we'll pretend we can for debuggering */

   if( (rv=ext_put(put_buf, address, 512)) != 0 )
   {
      switch(rv)
      {
      case 1:
	 printf("Parity error while copying to extended memory\n");
	 break;
      case 2:
	 printf("Interrupt error while copying to extended memory\n");
	 if ( tc>0 ) goto retry;
	 break;
      case 3:
	 printf("BIOS cannot open A20 Gate\n");
	 break;
      case 0x80: case 0x86:
	 printf("BIOS has no extended memory support\n");
	 break;
      default:
	 printf("Error %d while copying to extended memory\n", rv);
	 break;
      }
      return -1;
   }
   return 0;
}
#endif

static char *
read_cmdfile(iname)
char * iname;
{
   char * ptr = strchr(iname, '.');
   char buf[16];
   long len;

   buf[8] = '\0';
   strncpy(buf, iname, 8);
   strcat(buf, ".cfg");

   if( ptr == 0 && open_file(buf) >= 0 )
   {
      /* Ah, a command line ... */
      printf("Loading %s\n", buf);
      len = file_length();
      if( len > 1023 )
      {
	 printf("Command line file %s truncated to 1023 characters\n", buf);
	 len = 1023;
      }
      if( read_block(buffer) >= 0 )
      {
         register int i;
	 for(i=0; i<len; i++)
	    if( buffer[i] < ' ' ) buffer[i] = ' ';
	 buffer[len] = '\0';

         close_file();	/* Free up loads a room */
         return strdup(buffer);
      }
   }
   close_file();	/* Free up loads a room */
   return 0;
}

char *
input_cmd(iname)
char * iname;
{
   char lbuf[20];
   register int cc;

   for(;;)
   {
      printf("%s: ", iname); fflush(stdout);

      cc = read(0, buffer, sizeof(buffer)-1);
      if( cc <= 0 ) return 0;
      buffer[cc] = 0;
      if( cc == 1 && *buffer != '\n' )
      {
	 putchar('\n');
	 cc = (buffer[0] & 0xFF);

         if( cc == 0xAD ) /* ALT-X */
	    return 0;

#ifdef NOCOMMAND
         cmd_type("helpprmt.txt");
#else
	 help_key(cc);
#endif
	 continue;
      }
      if( buffer[cc-1] == '\n' ) buffer[cc-1] = '\0';

      break;
   }

   return strdup(buffer);
}

build_linuxarg(image, inp)
char *image, *inp;
{
static char * auto_str  = "auto";
static char * image_str = "BOOT_IMAGE=";
   int len = 0;
   char * ptr, *s, *d;
   char * free_cmd = 0, * cmd = 0;
   char * free_inp = 0;

   image_name = strdup(image);

   if( linux_command_line ) free(linux_command_line);
   linux_command_line = 0;

   if( cmd == 0 )
      cmd = free_cmd = read_cmdfile(image);

   if( cmd == 0 && auto_flag ) return 0;

   if( auto_flag )
   {
      ptr = strdup(cmd);
      for(s=strtok(ptr, " "); s; s=strtok((char*)0, " "))
      {
         if( strncasecmp(s, "prompt",6) == 0 && free_inp == 0)
            inp = free_inp = input_cmd(image);
      }
      free(ptr);
   }
   else if( inp == 0 )
   {
      inp = free_inp = input_cmd(image);
      if( inp == 0 ) 
      {
	 printf("\nAborted\n");
	 return -1;
      }
   }

   if( auto_flag ) len += strlen(auto_str) + 1;
   if( image ) len += strlen(image_str) + strlen(image) + 1;
   if( cmd ) len += strlen(cmd) + 1;
   if( inp ) len += strlen(inp) + 1;

   if( len == 0 ) return 0;

   ptr = malloc(len+1);
   if( ptr == 0 )
   {
      printf("Unable to create linux command line\n");
      if( free_cmd ) free(free_cmd);
      if( free_inp ) free(free_inp);
      return -1;
   }

   *ptr = 0; ptr[1] = 0;
   if( auto_flag ) { strcat(ptr, " "); strcat(ptr, auto_str); }
   if( cmd ) { strcat(ptr, " "); strcat(ptr, cmd); }
   if( inp ) { strcat(ptr, " "); strcat(ptr, inp); }

   if( free_cmd ) free(free_cmd);
   if( free_inp ) free(free_inp);

   if( (d = malloc(len+1)) == 0 )
   {
      printf("Unable to compact linux command line\n");
      free(ptr);
      return -1;
   }
   *d = '\0';
   for(s=strtok(ptr, " "); s; s=strtok((char*)0, " "))
   {
      if( strncasecmp(s, "vga=",4) == 0 )
      {
         if( strncasecmp(s+4, "ask", 3) == 0 )
	    vga_mode = -3;
	 else
         if( strncasecmp(s+4, "ext", 3) == 0 )
	    vga_mode = -2;
	 else
         if( strncasecmp(s+4, "nor", 3) == 0 )
	    vga_mode = -1;
	 else
         if( strncasecmp(s+4, "cur", 3) == 0 )
	    vga_mode = 0x0f04;
	 else
	 {
	    s+=4; getnum(&s, &vga_mode);
	 }
      }
      else if( strncasecmp(s, "ramdisk_file=", 13) == 0 )
      {
	 if( initrd_name ) free(initrd_name);
	 if( s[13] )
            initrd_name = strdup(s+13);
	 else
	    initrd_name = 0;
      }
      else if( strncasecmp(s, image_str, 11) == 0 )
      {
	 if( image_name ) free(image_name);
	 if( s[11] )
            image_name = strdup(s+11);
	 else
	    image_name = strdup(image);
      }
      else if( strncasecmp(s, "prompt",6) == 0 )
         ;
      else
      {
         strcat(d, " ");
	 strcat(d, s);
      }
   }
   free(ptr); ptr = d;

   strcat(ptr, " "); strcat(ptr, image_str); strcat(ptr, image_name);
   len = strlen(ptr);

   if( len > 2048 )
   {
      printf("Linux command line truncated to 2047 characters\n");
      ptr[2048] = 0;
      len = 2048;
   }

#ifdef __ELKS__
   fprintf(stderr, "Command line: '%s'\n", ptr+1);
#endif

   linux_command_line = ptr;

   return 0;
}

keep_going()
{
static int burning = 0;
   char buf[4];
   int cc;

   printf("Keep going ? "); fflush(stdout);

#ifdef __STANDALONE__
   cc = read(0, buf, 1);
#else
   cc = read(0, buf, 4);
#endif
   if( cc >= 1 && (*buf == 'Y' || *buf == 'y') )
   {
      printf("Yes, Ok%s\n", burning?"":", burn baby burn!");
      return burning=1;
   }
#ifdef NOCOMMAND
   printf("No, Ok press enter to reboot\n");
#else
   printf("No, Ok returning to monitor prompt\n");
#endif
   return 0;
}

load_initrd(k_top)
unsigned int k_top;
{
   unsigned int address, rd_start, rd_len;
   long file_len;
   char * fname = initrd_name;

   /* Top of accessable memory */
   if( main_mem_top >= 15360 ) address = 0xFFFF;
   else                        address = 0x1000 + main_mem_top*4;

   if( *initrd_name == '+' )
   {
      char buf[2];
      fname++;
      close_file();
      printf("Insert root disk and press return:"); fflush(stdout);
      if( read(0, buf, 2) <=0 ) return -1;
   }

   if( open_file(fname) < 0 )
   {
      printf("Cannot open %s\n", fname);
      return -1;
   }
   file_len = file_length();
   rd_len = (file_len+1023)/1024;

   if( file_len > 15000000L || k_top + rd_len*4 > address )
   {
      printf("Ramdisk file %s is too large to load\n", fname);
      return -1;
   }

   rd_start = address - rd_len*4;
   rd_start &= -16;	/* Page boundry */
   if (initrd_start && initrd_start<rd_start)
      rd_start = initrd_start;
   address = rd_start;

   printf("Loading %s at 0x%x00\n", fname, rd_start);

   for( ; rd_len>0 ; rd_len--)
   {
#ifndef NOCOMMAND
      int v = (kbhit()&0x7F);
      if( v == 3 || v == 27 )
      {
	 printf("User interrupt!\n");
         getch();
	 return -1;
      }
#endif

      printf("%dk to go \r", rd_len); fflush(stdout);
      if( read_block(buffer) < 0 )
      {
         printf("Read error loading ramdisk\n");
	 return -1;
      }
      if( putsect(buffer, address) < 0 ) return -1;
      address+=2;
      if( putsect(buffer+512, address) < 0 ) return -1;
      address+=2;
   }
   printf("          \r"); fflush(stdout);

   close_file();

   initrd_start  = ((long) rd_start <<8);
   initrd_length = file_len;

   return 0;
}

#ifdef CALC_CRC
check_crc()
{
   int low_sects;
   unsigned int address = 0x900;
   long len;
   int re_crc;

   reset_crc();

   __movedata(address*16, 0, __get_ds(), buffer, 512);
   low_sects = buffer[497] + 1; /* setup sects + boot sector */
   if (low_sects == 1) low_sects = 5;

   for(len=image_size; len>0; len-=512)
   {
      if( address >= 0xA00 )
      {
         if( ext_get(address, buffer, 512) != 0 )
	 {
	    printf("Unable to read back for CRC check\n");
	    return;
	 }
      }
      else
         __movedata(address*16, 0, __get_ds(), buffer, 512);

      if( len > 512 ) addcrc(buffer, 512); else addcrc(buffer, (int)len);
      
      address += 2;
      if( low_sects )
      {
	 low_sects--;
	 if( low_sects == 0 )
	 {
#if ZIMAGE_LOAD_SEG != 0x10000
	    if( is_zimage ) address = ZIMAGE_LOAD_SEG/16;
	    else            
#endif
	                    address = 0x1000;
	 }
      }
   }
   re_crc = display_crc("Images CRC check value =");

   if( re_crc != load_crc )
   {
      printf("Error: CRC doesn't match value written to memory!\n");
      return -1;
   }
   return 0;
}
#endif

#if ZIMAGE_LOAD_SEG == 0x10000
start_zimage()
{
#include "zimage.v"
   __movedata(__get_ds(), zimage_data, 0, zimage_start, zimage_size);
   {
#asm
  callf	zimage_start,0
#endasm
   }
}
#endif
