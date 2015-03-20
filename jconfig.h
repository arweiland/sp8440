/* 
 * Routines to access configuration files
 * Copyright (C) 2000-2002 Ariya Hidayat <ariyahidayat@yahoo.de>
 */

#ifndef JCONFIG_H
#define JCONFIG_H

typedef struct _JConfigVar JConfigVar;
struct _JConfigVar
{
  char* name;
  char* value;
  JConfigVar* next;
};

typedef struct _JConfigGroup JConfigGroup;
struct _JConfigGroup
{
  char* name;
  JConfigVar* vars;
  JConfigGroup* next;
};

struct _JConfig
{
  char* filename;
  JConfigGroup* groups;
};

typedef struct _JConfig JConfig;

/* opens a new config */
JConfig* jconfig_open( char* filename );

/* saves modifications to file */
void jconfig_save( JConfig* conf );

/* closes the config, doesn't save automatically  */
void jconfig_close( JConfig* conf );

/* returns number of groups found */
int jconfig_group_count( JConfig* conf );

/* returns name of group at position 'index' */
char* jconfig_group_name( JConfig* conf, int index );

/* returns non-zero if group is found or zero otherwise */
int jconfig_has_group( JConfig* conf, char* group );

/* adds a new group */
void jconfig_add_group( JConfig* conf, char* group );

/* removes a group and all its variables */
void jconfig_remove_group( JConfig* conf, char* group );

/* returns number of variables inside a group */
int jconfig_var_count( JConfig* conf, char* group  );

/* returns name of variable at position 'index' (within the given group) */
char* jconfig_var_name( JConfig* conf, char* group, int index );

/* returns non-zero if variable is found or zero otherwise */
int jconfig_has_var( JConfig* conf, char* group, char* name );

/* adds a new variable, the value will be still left to empty */
void jconfig_add_var( JConfig* conf, char* group, char* var );

/* removes a variable */
void jconfig_remove_var( JConfig* conf, char* group, char* var );

/* reads variable value as integer */
int jconfig_read_int( JConfig* conf, char* group, char* var, int def );

/* reads variable value as string */
char* jconfig_read_string( JConfig* conf, char* group, char* var, char* def );

/* sets a value of variable, will create the variable if necessary */
void jconfig_write_int( JConfig* conf, char* group, char* var, int value );

/* sets a value of variable, will create the variable if necessary */
void jconfig_write_string( JConfig* conf, char* group, char* var, char* value );

/* dumps all settings in configuration */
void jconfig_dump( JConfig* conf );

#endif /* JCONFIG_H */
