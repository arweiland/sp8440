/** 
 *  @file   main.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Main module for standalone testing
 *
 *  @section Description 
 * This file is only used for testing.\n
 * - If compiled as plugin, calls the plugin initialization code ripped from CLX\n
 * - If compiled as standalone, initialized the sp8440 module directly.\n
 */


#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "startup.h"
#include "plugins.h"

#ifndef PLUGIN
#include "msgQueue.h"
#endif

int (*alarm_8440)(char *msg, int alarm, int level);           // pointer to alarm function

int chk_sp8440Status( void );

void _test_loop( void );
int dummy( char *msg, int alarm, int level );
void register_sp8440_alarm( int (*fun)(char *msg, int alarm, int level) );

int main( void )
{
#ifdef PLUGIN
#include "plugins.h"
   int ret;

   printf( "Calling plugins_init\n" );
   // start via the plugins module
   plugins_init();
   printf( "Returned from plugins_init\n" );
   if ( (ret = chk_sp8440Status()) != PLUGIN_RUNNING )
   {
      printf( "%s: Plugin failed!  Code: %d\n", __func__, ret );
      return -1;
   }
   else
   {
      printf( "8440 Plugin started successfully\n" );
   }

#else
   // start directy
   printf( "Starting the module directly\n" );
   sp8440_Start( NULL );
#endif

//   register_sp8440_alarm( dummy );        // for test
//   (*alarm_8440)("hello!", 100, 2 );

   _test_loop();
   return 0;
}

/*---------- try to get the status of the SP8440 plugin --------*/

int chk_sp8440Status( void )
{
   plugin_t *pptr;
   int i;

   for ( i = 0, pptr = plugin_stats; i < MAX_PLUGINS; i++, pptr++ )
   {
      if ( strstr( pptr->descr, "SP8440" ) != NULL )
      {
         return pptr->status;
      }
   }

   return -1;           // not found
}


/*
 *  Test loop.  Output alarms to plugin every 30 seconds
 */
void _test_loop( void )
{
   sleep(1);

   while(1)
   {
      if ( alarm_8440 != NULL )
      {
         (*alarm_8440)( "Electrical", 100, 0);
         (*alarm_8440)( "Electrical", 100, 1);
         (*alarm_8440)( "Electrical", 100, 2);
      }
      else
      {
         printf( "SP8440 function call not registered!\n" );
         _exit(1);
      }
      sleep(30);
   }
}


/*
 * Register the function to call to add alarms to be sent to the phones.
 * This is the function that the CLX would call from the routing module
 */

void register_sp8440_alarm( int (*fun)(char *msg, int alarm, int level) )
{
   alarm_8440 = fun;           // save alarm function name
}

