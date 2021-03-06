/** 
 *  @file   startup.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/27/15
 *  @brief  Plugin main module for testing
 *
 *  @section Description 
 * 
 * - If the code is compiled as a plugin, loads the sp8440.so shared library and attempts to initialze it.\n
 *    It then goes into a loop outputting alert messages to all phones
 * - If compiled as standalone, initializes all modules and executes main loop
 */


#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "startup.h"
#include "spRec.h"
#include "server.h"
#include "msgSend.h"
#include "config.h"
#include "logging.h"
#include "plugins.h"
#include "msgQueue.h"

char *Version = "0.0.1 beta1";

void waitSec( int seconds );

pthread_mutex_t StartMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  StartCV = PTHREAD_COND_INITIALIZER;
pthread_t server_tid;

char PluginStatusString[100];
int  PluginStatus;

void *_main_thread( void *msg );

// CLX function to register msgQueue_Add function

extern void register_sp8440_alarm( int (*fun)(char *msg, int alarm, int level) );

/*---------( init_plugin )----------

  CLX plugin interface 

-----------------------------------*/

int init_plugin( plugin_t *stat )
{
   pthread_t tid;
   struct timespec ts;
   int ret;

#if 0
   // Do main module initialization
   if ( _main_Init() )
   {
      stat->status = PLUGIN_START_FAILED;
      return stat->status;
   }
#endif

   pthread_mutex_lock( &StartMutex );              // mke sure thread doesn't signal before we're ready

   printf( "SP8440: Starting Server\n" );
   pthread_create( &tid, NULL, sp8440_Start, NULL );   // Do startup (threaded for signaling)

   strcpy( stat->descr, "SP8440 Module" );        // copy description string
   strcpy( stat->version, Version );              // copy plugin version number

   // Wait until we get startup confirmation

   clock_gettime(CLOCK_REALTIME, &ts);
   ts.tv_sec += 2;                      // timeout (seconds)

   ret = pthread_cond_timedwait( &StartCV, &StartMutex, &ts );
   pthread_mutex_unlock( &StartMutex );

   if ( ret == ETIMEDOUT )              // upstream code failed to set conditional?
   {
	  stat->status = PLUGIN_START_FAILED;
	  strcpy( stat->statstr, "SP8440 Server timed out\n" );
   }
   else
   {
	  stat->status = (PluginStatus < 0) ? PLUGIN_START_FAILED : PLUGIN_RUNNING;
	  strcpy( stat->statstr, PluginStatusString );       // copy startup status string
   }

   if ( stat->status == PLUGIN_RUNNING )
   {
      register_sp8440_alarm( msgQueue_Add );          // provide function to call to add alarms
      // Start main plugin thread
      pthread_create( &tid, NULL, _main_thread, NULL );
   }

   return PluginStatus;
}


// Initialize the plugin and signal results

void *sp8440_Start( void *msg )
{
   pthread_t tid;

   if ( config_init( CFGNAME ) != 0 )
   {
      Log( CRIT, "%s: Can't open config file \"%s\"!  Terminating.\n", __func__, CFGNAME );
      printf( "%s: Can't open config file \"%s\"!  Terminating.\n", __func__, CFGNAME );
      MainSignal( -1, "Can't open config file!\n" );
      return NULL;
   }

   Log( INFO, "%s: Starting\n", __func__ );

   // Initialize the phones records module
   spRec_Init();

    // Start up server.  Calls MainSignal when started
   server_Init( &server_tid );

   // Start main plugin thread
   pthread_create( &tid, NULL, _main_thread, NULL );

#ifndef PLUGIN
   register_sp8440_alarm( msgQueue_Add );          // provide function to call to add alarms
#endif

   return NULL;
}


/*-------------( MainSignal )---------------

  Signal Main as to server startup status

  Set startup status and status string.

------------------------------------------*/

void MainSignal( int status, char *statstr )
{
   pthread_mutex_lock( &StartMutex );

   strcpy( PluginStatusString, statstr );
   PluginStatus = status;

   pthread_cond_signal( &StartCV );
   pthread_mutex_unlock( &StartMutex );
}

void *_main_thread( void *msg )
{
   while( 1 )
   {
      if ( pthread_kill( server_tid, 0 ) != 0 )    // check server thread
      {
         printf( "%s. Server dead. Exiting\n", __func__ );
         _exit(1);
      }
      spRec_CheckStale();    // check for "stale" phones
      sleep( 1 );
   }
}
