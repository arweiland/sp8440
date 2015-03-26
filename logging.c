/** 
 *  @file   logging.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Handles module logging duties
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#include "logging.h"
#include "config.h"


static int syslog_level;                  // current log level
static int syslog_rotate;                 // How many logs to rotate out
static int syslog_log_size;               // size of one log
static char syslog_file_path[100];        // name / path of syslog

static int phonelog_level;                // current log level
static int phonelog_rotate;               // How many logs to rotate out
static int phonelog_log_size;             // size of one log
static char phonelog_file_path[100];      // name / path of syslog


static char *lev_str[] =
{
   "EMER",
   "ALRT",
   "CRIT",
   "ERRO",
   "WARN",
   "NOTF",
   "INFO",
   "DBUG"
};

#if 0
static char *lev_str[] =
{
   "EMR",
   "ALT",
   "CRT",
   "ERR",
   "WAR",
   "NOT",
   "INF",
   "DBG"
};
#endif


#define SP8440_LOG_FILE  "spSystem"        // name of system log file, without extent
#define PHONES_LOG_FILE  "sp8440"          // name of phone log file, without extent

static void _Log_ChkRotate( char *fpath, int max_size );
static void _Log_rotate( char *fpath, int max_files );

/*---------  These would normally come from CLX's filesubs.c  ------------*/

#ifndef fpath_make_name
#define FSEP '/'
void fpath_make_name( char *dest, char *name, char *path )
{
   sprintf( dest, "%s%c%s", path, FSEP, name );
}
#endif

#ifndef fpath_get_path
#define LOG_PATH 2
char *fpath_get_path( int type )
{
   return "logs/";
}

#endif

int _Log_GetLevel( char *lstr );


/*----------------------------( PLog )-----------------------------

  Output a phone log to the log file with date / time stamp
  Rotate if time.

---------------------------------------------------------------*/

void PLog( int level, char *format, ... )
{
#define LOGF_BUF_SIZE  200

   char buffer[ LOGF_BUF_SIZE+1 ];
   FILE *fptr;
   va_list msg;
   int size;
   time_t curtime;
   char timeStr[100];
   char *log_level_str;
   char sname[100];
   struct tm *tmptr;

   va_start( msg, format );

   // If not set up yet, get configs

   if ( *phonelog_file_path == '\0' )
   {
      phonelog_log_size = config_readInt( "phones", "phonelog_size", 10 );
      phonelog_rotate = config_readInt( "phones", "phonelog_rotates", 4 );

      log_level_str = config_readStr( "phones", "phonelog_level", "INFO" );
      phonelog_level = _Log_GetLevel( log_level_str );

      // Get full log file name / path
      fpath_make_name( sname, PHONES_LOG_FILE, fpath_get_path( LOG_PATH ) );
      sprintf( phonelog_file_path, "%s.log", sname );      // full name with extent
   }

   // Get current time / date string
   curtime = time( NULL );           // Get current time
   tmptr = localtime( &curtime );
   strftime( timeStr, sizeof(timeStr), "%m/%d/%y %H:%M:%S", tmptr );

   // Get new log to output
   size = vsnprintf( buffer, LOGF_BUF_SIZE, format, msg );
   if ( size >= LOGF_BUF_SIZE )
   {
      buffer[ LOGF_BUF_SIZE-1 ] = '\0';
   }

   if ( phonelog_level >= level )
   {
      if ( log_to_stderr )  // output to error stream?
      {
         fprintf( stderr, "%s [%s] (phones) %s", timeStr, lev_str[ level ], buffer );
      }

      if ( phonelog_log_size > 0 )
      {
         if ( (fptr = fopen( phonelog_file_path, "a" )) != NULL )
         {
            fprintf( fptr, "%s [%s] %s", timeStr, lev_str[ level ], buffer );   // write out file
            fclose( fptr );

            // see if time to rotate the log file
            _Log_ChkRotate( phonelog_file_path, phonelog_log_size * 1000);
         }
      }
   }
}



/*----------------------------( Log )-----------------------------

  Output a log to the log file with date / time stamp
  Rotate if time.

---------------------------------------------------------------*/

void Log( int level, char *format, ... )
{
#define LOGF_BUF_SIZE  200

   char buffer[ LOGF_BUF_SIZE+1 ];
   FILE *fptr;
   va_list msg;
   int size;
   time_t curtime;
   char timeStr[100];
   char *log_level_str;
   char sname[100];
   struct tm *tmptr;

   va_start( msg, format );

   // If not set up yet, get configs

   if ( *syslog_file_path == '\0' )
   {
      syslog_log_size = config_readInt( "general", "syslog_size", 10 );
      syslog_rotate = config_readInt( "general", "syslog_rotates", 4 );

      log_level_str = config_readStr( "general", "syslog_level", "INFO" );
      syslog_level = _Log_GetLevel( log_level_str );

      // Get full log file name / path
      fpath_make_name( sname, SP8440_LOG_FILE, fpath_get_path( LOG_PATH ) );
      sprintf( syslog_file_path, "%s.log", sname );      // full name with extent
   }

   // Get current time / date string
   curtime = time( NULL );           // Get current time
   tmptr = localtime( &curtime );
   strftime( timeStr, sizeof(timeStr), "%m/%d/%y %H:%M:%S", tmptr );

   // Get new log to output
   size = vsnprintf( buffer, LOGF_BUF_SIZE, format, msg );
   if ( size >= LOGF_BUF_SIZE )
   {
      buffer[ LOGF_BUF_SIZE-1 ] = '\0';
   }

   if ( syslog_level >= level )
   {
      if ( log_to_stderr )  // output to error stream?
      {
         fprintf( stderr, "%s [%s] (system) %s", timeStr, lev_str[ level ], buffer );
      }

      if ( syslog_log_size > 0 )
      {
         if ( (fptr = fopen( syslog_file_path, "a" )) != NULL )
         {
            fprintf( fptr, "%s [%s] %s", timeStr, lev_str[ level ], buffer );   // write out file
            fclose( fptr );

            // see if time to rotate the log file
            _Log_ChkRotate( syslog_file_path, syslog_log_size * 1000);
         }
      }
   }
}


/*--------------------------( _Log_ChkRotate )---------------------------------

   Rotate the log files
   fname is the name of the file with path but without extent

---------------------------------------------------------------------------*/

static void _Log_ChkRotate( char *fpath, int max_size )
{
   struct stat statf;

   if ( stat( fpath, &statf ) == 0 )            // log file exists?
   {
      if ( statf.st_size > max_size )           // now too big?
      {
         _Log_rotate( fpath, max_size );        // rotate the logs
      }
   }
}



/*--------------------------( _Log_rotate )---------------------------------

   Rotate the log files
   fname is the name of the file with path but without extent

---------------------------------------------------------------------------*/

static void _Log_rotate( char *fpath, int max_files )
{
   char old_file[ 80 ];
   char new_file[ 80 ];
   int i;
   FILE *fptr;

   // delete oldest file if it exists

   sprintf( old_file, "%s.%d", fpath, max_files );

   if ( access( old_file, F_OK )  == 0 )     // oldest exist?
   {
      unlink( old_file );                    // to the bit-bucket
   }

   // move all others down by one

   for ( i = max_files-1; i; i-- )
   {
      sprintf( old_file, "%s.%d", fpath, i );
      sprintf( new_file, "%s.%d", fpath, i+1 );

      if ( access( old_file, F_OK ) == 0 )  // if old file exists
      {
         rename( old_file, new_file );      // rename it
      }
   }

   // now rotate out current log file

   sprintf( new_file, "%s", fpath );
   rename( new_file, old_file );

   // create a new empty log file

   fptr = fopen( new_file, "a" );
   fclose( fptr );

}


/*---------------------------( _Log_getLevel )-------------------------


--------------------------------------------------------------------*/

static char *log_levels[] = 
{
   "emer",          // 0
   "alert",         // 1
   "crit",          // 2
   "error",         // 3
   "warn",          // 4
   "notice",        // 5
   "info",          // 6
   "debug"          // 7
};

#define N_LOG_LEVELS  (sizeof (log_levels) / sizeof( log_levels[0] ))

int _Log_GetLevel( char *lstr )
{
   char **str;
   int i;

   for( i = 0, str = log_levels; i < N_LOG_LEVELS; i++, str++ )
   {
      if ( strcasestr( lstr, *str ) != NULL )
      {
         return i;
      }
   }

   return INFO;       // not found. Use INFO level
}

