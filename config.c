/** 
 *  @file   config.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Helper functions for configuration parser
 *
 *
 */

#include <stdio.h>

#include "jconfig.h"
#include "config.h"

static JConfig *cfgPtr;

// If the following is true, enable some system test stuff (like log output to stderr)
int log_to_stderr = 0;

int config_init( char *cfgName )
{
   FILE *fptr;

   // first test if config file if openable.
   // Jconfig doesn't tell us.

   if ( (fptr = fopen( cfgName, "r" )) == NULL )
   {
      return 1;
   }

   fclose( fptr );
   cfgPtr = jconfig_open( cfgName );      // open configuration file

   // Check for system test mode
   log_to_stderr = config_readInt( "general", "log_to_stderr", 0 );

   return 0;
}


int config_readInt( char *group, char *var, int def )
{
   if ( cfgPtr == NULL )
   {
      return def;
   }
   return jconfig_read_int( cfgPtr, group, var, def );
}

char *config_readStr( char *group, char *var, char *def )
{
   char *str;
   if ( cfgPtr == NULL )
   {
      return def;
   }
   str = jconfig_read_string( cfgPtr, group, var, def );
   return str;
}

