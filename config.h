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

extern int log_to_stderr;

int config_init( char *cfgName );
int config_readInt( char *group, char *var, int def );
char *config_readStr( char *group, char *var, char *def );

#endif
