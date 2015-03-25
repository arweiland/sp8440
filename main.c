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

int main( void )
{
   pthread_t tid;

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
   msgSend_PushAlert( "Electrical", 100, 2);

   while( 1 )
   {
      sleep(1);                     // sleep for a second
      if ( pthread_kill( tid, 0 ) != 0 )    // check server thread
      {
         return 0;                          // server thread died
      }
      spRec_CheckStale();                   // check for "stale" phones
   }
   // join on web server thread
//   pthread_join( tid, NULL );
   return 0;
}

