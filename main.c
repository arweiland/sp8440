#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "startup.h"
#include "plugins.h"

int chk_sp8440Status( void );

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
      printf( "8440 Pluin started successfully\n" );
   }

#else
   // start directy
   printf( "Starting the module directly\n" );
   sp8440_Start();
#endif

   while( 1 )
   {
      sleep(1);
   }
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

