
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#include "logging.h"
#include "config.h"


static int syslog_level;                // current log level
static int syslog_rotate;               // How many logs to rotate out
static int syslog_log_size;             // size of one log
static char syslog_file_path[100];      // name / path of syslog

char *lev_str[] =
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

#define SP8440_LOG_FILE  "sp8440"        // name of log file, without extent

void _Log_rotate( char *fpath, int max_files );

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
   return ".";
}

#endif

int _Log_GetLevel( char *lstr );


/*----------------------------( _Log )-----------------------------

  Output a log to the log file with date / time stamp
  Rotate if time.

---------------------------------------------------------------*/

void Log( int level, char *format, ... )
{
#define LOGF_BUF_SIZE  200

   char buffer[ LOGF_BUF_SIZE+1 ];
   FILE *fptr;
   struct stat statf;
   va_list msg;
   int size;
   time_t curtime;
   char timeStr[100];
   char *syslog_level_str;
   char sname[100];
   struct tm *tmptr;

   char *lev_str[] =
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

   va_start( msg, format );

   // If not set up yet, get configs

   if ( *syslog_file_path == '\0' )
   {
      syslog_log_size = config_readInt( "general", "log_size", 1000 );
      syslog_rotate = config_readInt( "general", "n_logs", 4 );

      syslog_level_str = config_readStr( "general", "log_level", "INFO" );
      syslog_level = _Log_GetLevel( syslog_level_str );

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

   if ( syslog_level >= level && syslog_log_size > 0 )
   {
      if ( (fptr = fopen( syslog_file_path, "a" )) != NULL )
      {
         fprintf( fptr, "%s [%s] %s", timeStr, lev_str[ level ], buffer );   // write out file

         fclose( fptr );

         // see if time to rotate the log file

         if ( stat( syslog_file_path, &statf ) == 0 )                    // log file exists?
         {
            if ( statf.st_size > (syslog_log_size * 1000) )          // now too big?
            {
               _Log_rotate( syslog_file_path, syslog_rotate );       // rotate the logs
            }
         }
      }
   }
}


/*--------------------------( _Log_rotate )---------------------------------

   Rotate the log files
   fname is the name of the file with path but without extent

---------------------------------------------------------------------------*/

void _Log_rotate( char *fpath, int max_files )
{
   char old_file[ 80 ];
   char new_file[ 80 ];
   int i;
   FILE *fptr;

   // delete oldest file if it exists

   sprintf( old_file, "%s.log.%d", fpath, max_files );

   if ( access( old_file, F_OK )  == 0 )     // oldest exist?
   {
      unlink( old_file );                    // to the bit-bucket
   }

   // move all others down by one

   for ( i = max_files-1; i; i-- )
   {
      sprintf( old_file, "%s.log.%d", fpath, i );
      sprintf( new_file, "%s.log.%d", fpath, i+1 );

      if ( access( old_file, F_OK ) == 0 )  // if old file exists
      {
         rename( old_file, new_file );      // rename it
      }
   }

   // now rotate out current log file

   sprintf( new_file, "%s.log", fpath );
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

