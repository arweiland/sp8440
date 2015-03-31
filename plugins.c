/*==========================================================================
|                                                                           |
|   file:   PLUGINS.C                       CLX plugin library handler      |
|                                                                           |
|   author:     Ron Weiland                 date:  17-Nov-09                |
|                                                                           |
|   current version:    1.00                                                |
|                                                                           |
|                                                                           |
| ver    sys    date               modification                         by  |
| ---    ---    ----               ------------                         --  |
|                                                                           |
| 1.00        17-Nov-09 Created                                         ARW |
|                                                                           |
==========================================================================*/

#include <dirent.h> 
#include <stdio.h> 
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "plugins.h"
// #include "warnings.h"

//char *plugins_dir = "/usr/shoptalk/plugins";
char *plugins_dir = ".";

void plugins_open( char *lib_name, plugin_t *status );

plugin_t plugin_stats[ MAX_PLUGINS ];   // array of plugin status



/*-------------------( plugins_init )----------------------

  Scan the plugins directory for any library files.
  If found, load and initialize them

---------------------------------------------------------*/

void plugins_init( void )
{

   DIR           *d;
   struct dirent *dir;
   char *ext;
   int  plugin_number = 0;
   char buf[ 100 ];

   d = opendir( plugins_dir );

   // try to open the plugins directory

   if (d == NULL )
   {
	  printf( "Can't open %s directory\n", plugins_dir );
	  return;
   }

   printf( "\n" );

   // look for files with the .so extension

   while ((dir = readdir(d)) != NULL)
   {
	  if ( (ext = strrchr( dir->d_name, '.' )) != NULL )   // point dot
	  {
		 if ( !strcmp( ext, ".so" ) )    // look for a ".so" file
		 {
			strcpy( plugin_stats[ plugin_number ].fname, dir->d_name );
			sprintf( buf, "%s/%s", plugins_dir, dir->d_name );
			plugins_open( buf, &plugin_stats[ plugin_number++ ] );
		 }
	  }
   }

   closedir(d);
}


/*-------------------( plugins_open )----------------------

  Try to open and initialize a plugin library

---------------------------------------------------------*/

void plugins_open( char *lib_name, plugin_t *status )
{
   int (*init)( plugin_t *status );
   void *dlLib;
   char *err;
   struct stat fstat;
   int rstat;
   int ret;

   if ( stat( lib_name, &fstat ) < 0 )
   {
      status->status = PLUGIN_OPEN_FAILED;
      strcpy( status->statstr, strerror( errno ) );
      perror( status->statstr );
      return;
   }

   // try to open the library
   /* if RTLD_NOW flag is set, the system will die if there
      are undefined symbols in the plugin.  If RTLD_LAZY is
      set, the system will output an "can't resolve symbol"
      string to stderr.
   */

//   dlLib = dlopen( lib_name, RTLD_GLOBAL | RTLD_NOW );
   dlLib = dlopen( lib_name, RTLD_GLOBAL | RTLD_LAZY );

   if ( (err = dlerror()) != NULL )
   {
      status->status = PLUGIN_OPEN_FAILED;
      strncpy( status->statstr, err, sizeof( status->statstr) );
      return;
   }

   dlerror();       // clear any error string

   // try to find initialization routine

   init = dlsym(dlLib, "init_plugin");

   if ((err = dlerror()) != NULL) 
   {   
      status->status = PLUGIN_OPEN_FAILED;
      strncpy( status->statstr, err, sizeof( status->statstr ) );
      return;
   }

   // initialize plugin, get status

   rstat = (*init)( status ); 

   printf( "Plugin %s returned status %d, string: %s\n",status->fname, rstat, status->statstr ); 
   printf( "Plugin description: %s, version: %s\n", status->descr, status->version );

   if ( rstat < 0 )
   {
      status->status = PLUGIN_START_FAILED;
      printf( "Unloading plugin %s. Returned status: %d\n", lib_name, rstat );
      ret = dlclose( dlLib );
      printf( "dlclose returned %d\n", ret );
   }
   else
   {
      status->status = PLUGIN_RUNNING;
      status->handle = dlLib;
      printf( "Plugin %s running\n", status->fname );
   }

   printf( "\n" );
}

