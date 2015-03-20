
#ifndef _OURCONFIG_H_
#define _OURCONFIG_H_

int config_init( char *cfgName );
int config_readInt( char *group, char *var, int def );
char *config_readStr( char *group, char *var, char *def );

#endif
