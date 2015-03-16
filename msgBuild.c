/** 
 *  @file msgBuild.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  phone alert msg builder
 *
 *  @section DESCRIPTION 
 * 
 * 
 * Using HTML template, creates message ready to send to phone
 *
 */


/*
 *  Note:  The message template MUST contain:
 *  two %s in <hr style for the line color,
 *  a "%s" for the escalation number
 *  a "%d" for the alarm number
 *  a %s for the alarm message to the phone
 */


#include <stdio.h>
#include "msgBuild.h"

#define MAXFILE 2000

int msgBuild_makePushMsg( char *template_fname, char *outbuf, int bufsize, int alarm_num, char *alarm_msg, int level )
{
   char buf[MAXFILE];
   FILE *fptr;
   int len;
   char *color;
   char *lnone = "\0";
   char *lptr;

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

   switch( level )
   {
      case 0:
         lptr = lnone;                      // no "Request" message
         color = "green";
         break;
      case 1:
         lptr = "2nd Request";
         color = "yellow";
         break;
      case 2:
      default:
         lptr = "3rd Request";
         color = "red";
         break;
   }
   
   // Create completed HTML output data
   if ( (len = snprintf( outbuf, bufsize, buf, color, color, alarm_num, lptr, alarm_msg )) >= bufsize )
   {
      printf( "file \"%s\" is too long.  Can't be over %d bytes. Is: %d bytes\n", template_fname, MAXFILE, len );
      fclose( fptr );
      return -1;
   }

   printf( "%s\n", outbuf );
//   printf( "Message size: %d\n", len );
   fclose( fptr );
   return len;
}
