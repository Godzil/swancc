char testy[] = "Blablablab42";

void trap()
{
   write( 1, "Ouch!!\n", 7);
}

int main(argc, argv)
{
   char buf[2];
   int cc;

   signal(SIGINT, trap);
   while( (cc=read(0, buf, 1)) > 0 || (cc == -1 && errno == EINTR) )
   {
      if( cc < 0 )
      {
          fprintf(stderr, "INTR\n");
      }
      else
      {
          fprintf(stderr, "%x\n", buf[0]);
      }
   }

   
   write(1, "\nExit!\n", 7);
}
