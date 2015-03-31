/** 
 *  @file   config.h
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Helper functions for configuration parser, header file
 *
 *
 */

#ifndef _OURCONFIG_H_
#define _OURCONFIG_H_

/*------- define the base directory for sp8440 files ------*/
// These are relative to the main CLX directory

#define CFGNAME "data/sp8440.cfg"       // configuration file
#define BASEDIR "data/sp8440/"          // directory for HTML files, .wav, etc
#define LOGDIR "logs/"

extern int log_to_stderr;

int config_init( char *cfgName );
int config_readInt( char *group, char *var, int def );
char *config_readStr( char *group, char *var, char *def );

#endif
