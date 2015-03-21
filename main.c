/** 
 *  @file   main.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Plugin main modle
 *
 *  @section Description 
 * 
 * Initializes all modules.\n
 * Starts web server and waits until it dies
 */


#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "spRec.h"
#include "server.h"
#include "msgSend.h"
#include "config.h"

char *cfgFile = "sp8440.cfg";

int main( void )
{
   pthread_t tid;

   if ( config_init( cfgFile ) != 0 )
   {
      printf( "Can't open config file \"%s\"!  Terminating.\n", cfgFile );
      return 1;
   }

   spRec_Init();
   server_Init( &tid );
   sleep(1);
   msgSend_PushAlert( "Electrical", 2);

   // join on web server thread
   pthread_join( tid, NULL );
   return 0;
}

