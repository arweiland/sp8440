/** 
 *  @file   main.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Plugin main module for testing
 *
 *  @section Description 
 * 
 * Initializes all modules.\n
 * Starts web server and waits until it dies
 */


#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "spRec.h"
#include "server.h"
#include "msgSend.h"
#include "config.h"
#include "logging.h"

char *cfgFile = "sp8440.cfg";

static pthread_t tid;

void wait( int seconds );

int main( void )
{

   if ( config_init( cfgFile ) != 0 )
   {
      printf( "Can't open config file \"%s\"!  Terminating.\n", cfgFile );
      return 1;
   }

   Log( INFO, "%s: Starting\n", __func__ );

   if ( spRec_Init() )
   {
      return 1;
   }

   server_Init( &tid );
   sleep(1);

   while(1)
   {
      // Wait until there is at least one phone to output to
      while ( spRec_GetNextRecord( NULL ) == NULL )
      {
         sleep( 1 );
      }
      msgSend_PushAlert( "Electrical", 100, 0);
      wait( 30 );
      msgSend_PushAlert( "Electrical", 100, 1);
      wait( 30 );
      msgSend_PushAlert( "Electrical", 100, 2);
      wait( 30 );
   }

   // join on web server thread
//   pthread_join( tid, NULL );
   return 0;
}

void wait( int seconds )
{
   int i;
   for( i = 0; i < seconds; i++ )
   {
      sleep(1);
      if ( pthread_kill( tid, 0 ) != 0 )    // check server thread
      {
         _exit(1);
      }
      spRec_CheckStale();    // check for "stale" phones
   }
}
