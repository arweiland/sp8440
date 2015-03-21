/** 
 *  @file msgBuild.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  phone HTML msg builder
 *
 *  @section Description
 * 
 * 
 * Using HTML template and parameters, creates messages ready to send to phone
 *
 */


#include <stdio.h>
#include <string.h>

#include "msgBuild.h"
#include "strsub.h"
#include "server.h"

typedef struct
{
   char *match;       // token to match in HTML file
   char **replace;    // pointer to string to replace with
}replace_def_t;

static char alarmStr[10];

static char *barColor;
static char *server;
static char *deptName;
static char *alarmNum;
static char *levelStr;

replace_def_t replacements[] =
{
   {"[COLOR]",  &barColor },         // color of bar to put, top and bottom
   {"[SERVER]", &server },           // server IP and port
   {"[DEPT]",   &deptName },         // department name
   {"[ALARM]",  &alarmNum },         // alarm number
   {"[LEVEL]",  &levelStr },         // string, based on escalation level
   {NULL,NULL}                       // end of substituion list
};


void _msgBuild_buildMsg( char *outbuf, char *template, int maxLen );
int _msgBuild_ReadTemplate( char *template_fname, char *buf );


#define MAXFILE 2000            // This is the largest amount of data the phone will accept


int msgBuild_makeAlertMsg( char *template_fname, char *outbuf, int bufsize, char *dept, int alarm_num, int level )
{
   char template[MAXFILE];
   char *lnone = "\0";
   int ret;

   if ( (ret = _msgBuild_ReadTemplate( template_fname, template )) != 0 )
   {
      return ret;         // reading template failed
   }

   switch( level )
   {
      case 0:
         levelStr = lnone;                    // no "Request" message
         barColor = "green";
         break;
      case 1:
         levelStr = "2nd Request";
         barColor = "yellow";
         break;
      case 2:
      default:
         levelStr = "3rd Request";
         barColor = "red";
         break;
   }
   deptName = dept;
   sprintf(alarmStr, "%d", alarm_num);      // get alarm number as string
   server = server_GetOurAddress();         // get system address:port

   _msgBuild_buildMsg( outbuf, template, MAXFILE );

//   printf( "%s\n", outbuf );
   printf( "Message size: %ld\n", strlen( outbuf ) );
   return 0;
}



int msgBuild_makeAcceptMsg( char *template_fname, char *outbuf, int bufsize, char *dept, char *msg )
{
   char template[MAXFILE];
   int ret;

   if ( (ret = _msgBuild_ReadTemplate( template_fname, template )) != 0 )
   {
      return ret;         // reading template failed
   }

   deptName = dept;
   levelStr = msg;
   server = server_GetOurAddress();         // get system address:port

   _msgBuild_buildMsg( outbuf, template, MAXFILE );

   // Create completed HTML output data
//   if ( (len = snprintf( outbuf, bufsize, buf, dept, msg )) >= bufsize )
//   {
//      printf( "file \"%s\" is too long.  Can't be over %d bytes. Is: %d bytes\n", template_fname, MAXFILE, len );
//      return -1;
//   }

//   printf( "%s\n", outbuf );
   printf( "Message size: %ld\n", strlen( outbuf ) );
   return 0;

}

void _msgBuild_buildMsg( char *outbuf, char *template, int maxLen )
{
   char buf2[ maxLen ];
   char *orig = template;
   char *dest = buf2;
   char *tptr;
   int first = 1;
   replace_def_t *rptr = replacements;

   while( rptr->match != NULL )
   {
      strsub_Replace( dest, orig, rptr->match, *rptr->replace );     // replace matches in message

      if ( first )
      {
         orig = outbuf;                 // only use template on first pass
         first = 0;
      }

      // ping-pong buffers
      tptr = dest; dest = orig; orig = tptr;
      rptr++;
   }

   if ( orig != outbuf )
   {
      strcpy( outbuf, buf2 );         // move to output
   }
}



int _msgBuild_ReadTemplate( char *template_fname, char *buf )
{
   FILE *fptr;
   int len;

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
   fclose( fptr );

   return 0;
}
