/*
 *  Note:  The message file must contain a "%d" for the alarm number, and then a %s for the alarm message to the phone,
 *  then two %s in <hr tag for the line color
 */


#include <stdio.h>
#include "msgBuild.h"

#define MAXFILE 2000

int msgBuild_makePushMsg( char *template_fname, char *outbuf, int bufsize, int alarm_num, char *alarm_msg )
{
   char buf[MAXFILE];
   FILE *fptr;
   int len;
   char *color = "yellow";

   if ( (fptr = fopen( template_fname, "r" )) == NULL )
   {
      printf( "Can't open file \"%s\"\n", template_fname );
      return -1;
   }

   // Read in the template file to use
   len = fread( buf, 1, MAXFILE, fptr );     // read in the form
   if ( len >= MAXFILE )
   {
      printf( "file \"%s\" is too long.  Can't be over %d bytes\n", template_fname, MAXFILE );
      fclose( fptr );
      return -1;
   }
   buf[len] = '\0';                         // null-terminate the template string

   // Create completed HTML output data
   if ( (len = snprintf( outbuf, bufsize, buf, alarm_num, alarm_msg, color, color )) >= bufsize )
   {
      printf( "file \"%s\" is too long.  Can't be over %d bytes\n", template_fname, MAXFILE );
      fclose( fptr );
      return -1;
   }

//   printf( "%s\n", outbuf );
//   printf( "Message size: %d\n", len );
   fclose( fptr );
   return len;
}
