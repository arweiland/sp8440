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
#include "config.h"
#include "logging.h"

static char alarmStr[10];      // Alarm number string
static char audio1Str[20];     // name of audio file 1
static char audio2Str[20];     // name of audio file 2
static char audio3Str[20];     // name of audio file 3

static char *barColor;
static char *server;
static char *deptName;
static char *alarmNum = alarmStr;
static char *audio1 = audio1Str;
static char *audio2 = audio2Str;
static char *audio3 = audio3Str;
static char *levelStr;

typedef struct
{
   char *match;       // token to match in HTML file
   char **replace;    // pointer to string to replace with
}replace_def_t;

replace_def_t replacements[] =
{
   {"[COLOR]",  &barColor },         // color of bar to put, top and bottom
   {"[SERVER]", &server },           // server IP and port
   {"[DEPT]",   &deptName },         // department name
   {"[ALARM]",  &alarmNum },         // alarm number
   {"[LEVEL]",  &levelStr },         // string, based on escalation level
   {"[AUDIO1]", &audio1 },           // First audio file
   {"[AUDIO2]", &audio2 },           // Second audio file
   {"[AUDIO3]", &audio3 },           // Third audio file
   {NULL,NULL}                       // end of substituion list
};


void _msgBuild_buildMsg( char *outbuf, char *template, int maxLen );
int _msgBuild_ReadTemplate( char *template_fname, char *buf );
void _msgBuild_ChkAudio( void );

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

   Log( DEBUG, "%s: Message size: %d\n", __func__, (int)strlen( outbuf ) );
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

//   printf( "%s\n", outbuf );
   Log( DEBUG, "%s: Message size: %d\n", __func__, (int)strlen( outbuf ) );
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

   _msgBuild_ChkAudio();                // Make sure audio file names have been read from config

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

// Read in the given template file

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


// Check the audio messages from the config file

void _msgBuild_ChkAudio( void )
{
   char *str;

   if ( *audio1Str == '\0' )     // Not read from config yet?
   {
      str = config_readStr( "phones", "audio1", "audio1.wav" );
      strncpy( audio1Str, str, sizeof( audio1Str ) );
   }

   if ( *audio2Str == '\0' )     // Not read from config yet?
   {
      str = config_readStr( "phones", "audio2", "audio2.wav" );
      strncpy( audio2Str, str, sizeof( audio2Str ) );
   }

   if ( *audio3Str == '\0' )     // Not read from config yet?
   {
      str = config_readStr( "phones", "audio3", "audio3.wav" );
      strncpy( audio3Str, str, sizeof( audio3Str ) );
   }
}
