/**
 *  @file   strsub.c
 *  @author Ron Weiland, Indyme Solutions
 *  @brief  String substitution module
 *  
 */


#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>


#if 0
int main( void )
{
   char inbuf[2000];        // input template buffer
   char buf1[ 2000 ];
   char buf2[ 2000 ];
   int len;
   FILE *fp;
   int i;

   // Get data to process
   len = read( 0, inbuf, sizeof( inbuf ) );
   inbuf[len] = '\0';
 
   for ( i = 0; i < 1000; i++ )
   {
      strsub_Replace( buf1, inbuf, "[COLOR]", "red" );
      strsub_Replace( buf2, buf1, "[SERVER]", "192.168.1.138:8080" );
      strsub_Replace( buf1, buf2, "[DEPT]", "Electrical" );
      strsub_Replace( buf2, buf1, "[ALARM]", "100" );
      strsub_Replace( buf1, buf2, "[LEVEL]", "2nd Request" );
   }

   // write out to file
   fp = fopen( "output.dat", "w" );
   fputs( buf1, fp );
   fclose( fp );

   return 0;
}
#endif

char *strsub_Replace( char *dest, char *orig, char *rep, char *with) 
{
   char *result; // the return string
   char *ins;    // the next insert point
   char *tmp;    // varies
   int len_rep;  // length of rep
   int len_with; // length of with
   int len_front; // distance between rep and end of last rep
   int count;    // number of replacements

   if (!orig)
      return NULL;
   if (!rep)
      rep = "";
   len_rep = strlen(rep);
   if (!with)
      with = "";
   len_with = strlen(with);

   ins = orig;
   for (count = 0; (tmp = strstr(ins, rep)); ++count) {
      ins = tmp + len_rep;
   }

   // first time through the loop, all the variable are set correctly
   // from here on,
   //    tmp points to the end of the result string
   //    ins points to the next occurrence of rep in orig
   //    orig points to the remainder of orig after "end of rep"
//   tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);
   tmp = result = dest;

   if (!result)
      return NULL;

   while (count--) {
      ins = strstr(orig, rep);
      len_front = ins - orig;
      tmp = strncpy(tmp, orig, len_front) + len_front;
      tmp = strcpy(tmp, with) + len_with;
      orig += len_front + len_rep; // move to next "end of rep"
   }
   strcpy(tmp, orig);
   return result;
}
