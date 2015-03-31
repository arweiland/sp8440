/*==========================================================================
|                                                                           |
|   file:   PLUGINS.H                       CLX plugin library handler      |
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

#ifndef _PLUGINS_H_
#define _PLUGINS_H_

typedef enum
{
   PLUGIN_NONE,                 // no plugin
   PLUGIN_OPEN_FAILED,          // plugin failed to start
   PLUGIN_START_FAILED,         // failed to start
   PLUGIN_RUNNING,              // plugin running
   PLUGIN_SHUTDOWN,             // plugin shut down
}PLUGIN_STATUS;


typedef struct
{
	  PLUGIN_STATUS  status;   // current status of plugin
	  char fname[100];         // plugin file name
	  char descr[100];         // short description
	  char statstr[100];       // plugin status return string
	  char version[10];        // plugin version (major.minor)
	  void *handle;            // handle returned by dlopen
}plugin_t;


#define MAX_PLUGINS   10       // max number of plugins allowed

extern plugin_t plugin_stats[ MAX_PLUGINS ];   // array of plugin status


void plugins_init( void );

#endif
