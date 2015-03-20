/*
 * Routines to access configuration files
 * Copyright (C) 2000-2002 Ariya Hidayat <ariyahidayat@yahoo.de>
 */

#include "jconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>



/* prototypes for internal helper functions */
JConfigGroup* jconfig_find_group( JConfig* conf, char* group );
JConfigVar* jconfig_find_var( JConfig* conf, char* group, char* var );
char* jconfig_strip_spaces( char* string );

/* for development and debugging */
static int jmalloc_count = 0;

void* jmalloc( int size )
{
  jmalloc_count++;
  return malloc( size );
}

void jfree( void* ptr )
{
  if( !ptr ) return;
  free( ptr );
  jmalloc_count--;
}

char* jstrdup( char* string )
{
  jmalloc_count++;
  return strdup( string );
}

/*==================================================================*/
JConfig* jconfig_open( char* filename )
{
  JConfig* conf = (JConfig*) 0L;
  FILE *f;
  char buffer[256], *p, *line;
  char* group, *var, *value;

  var = value = (char*) 0L;

#ifdef DEBUG
  printf( "jconfig_open [%s]\n", filename );
#endif

  /* create a new instance */
  conf = (JConfig*) jmalloc( sizeof( JConfig ) );
  conf->filename = jstrdup( filename );
  conf->groups = 0;

  /* load first from file if possible */
  f = fopen( filename, "rt" );
  if( !f )
  {
    syslog( LOG_WARNING, "jconfig: can't load from file %s !\n", filename );
    return conf;
  }

  /* default to empty group, means global */
  group = strdup("");

  while( !feof( f ) )
  {
    fgets( buffer, sizeof(buffer)-1, f );
    buffer[ sizeof(buffer)-1 ] = '\0';

    /* remove useless trailing CR LF */
    if( strlen( buffer ) > 0 )
    {
      p = buffer + strlen( buffer ) - 1;
      while( (p >=buffer ) && ( *p==10 || *p==13 ) ) *p--= '\0';
    }

    /* find char '#' as marker for comment */
    for( p = buffer; *p; p++ )
      if( *p=='#' )
      {
        *p = '\0';
        break;
      }

    /* strip whitespaces, only process non-empty line */
    line = jconfig_strip_spaces( buffer );
    /* printf( "parsing: %s\n", line ); */

    if( line )
    {

      /* define a group? */
      if( line[0] == '[' )
      {
        if( line[ strlen(line)-1 ] == ']' )
        {
          line[ strlen(line)-1 ] = '\0'; /* exclude ']' */
          jfree( group );
          group = jstrdup( line + 1 );  /* exclude '[' */
          jconfig_add_group( conf, group );
        }
      }

      else
      {
        /* look for '=' assignment */
        for( p = line; *p; p++ )
          if( *p == '=' )
          {
            *p = '\0';
            var = jconfig_strip_spaces( line );
            value = jconfig_strip_spaces( *(p+1) ? p + 1 : p );

            /* looks good so far, add as a new variable */
            if( var ) jconfig_write_string( conf, group, var, value );

            jfree( value );
            jfree( var );

            break;
          }
      }
    }

    jfree( line );
  }

  jfree( group );

  fclose( f );

  return conf;
}

/*==================================================================*/
void jconfig_save( JConfig* conf )
{
  FILE* f;
  JConfigGroup* group;
  JConfigVar* var;

  if( !conf ) return;

  if( !conf->filename )
  {
    printf( "warning: jconfig_save() without filename !\n" );
    return;
  }

#ifdef DEBUG
  printf( "jconfig_save [%s]\n", conf->filename );
#endif

  f = fopen( conf->filename, "wt" );
  if( !f )
  {
    fprintf( stderr, "error: can't save to %s\n", conf->filename );
    return;
  }

  /* first of all, save global variables, i.e without group name */
  group = jconfig_find_group( conf, "" );
  if( group )
    for( var = group->vars; var; var = var->next )
      fprintf( f, "%s = %s\n", var->name, var->value );

  /* save all other groups */
  for( group = conf->groups; group; group = group->next )
  {
    if( strlen( group->name )==0 ) continue; /* skip global */
    fprintf( f, "\n[%s]\n", group->name );
    for( var = group->vars; var; var = var->next )
      fprintf( f, "%s = %s\n", var->name, var->value );
  }

  fclose( f );

}

/*==================================================================*/
void jconfig_close( JConfig* conf )
{
  JConfigGroup *group;

  if( !conf ) return;

#ifdef DEBUG
  printf( "jconfig_close [%s]\n", conf->filename );
#endif

  /* remove all groups */
  group = conf->groups;
  while( group )
  {
    jconfig_remove_group( conf, group->name );
    group = conf->groups;
  }

  /* remove itself */
  jfree( conf->filename );
  jfree( conf );

  if( jmalloc_count > 0 )
    fprintf( stderr, "error: mem leak %d !\n", jmalloc_count );
}

/*==================================================================*/
int jconfig_group_count( JConfig* conf )
{
  int count = 0;
  JConfigGroup* cgroup;

  if( !conf ) return 0;

  for( cgroup = conf->groups; cgroup; cgroup = cgroup->next )
    count++;

  return count;
}

/*==================================================================*/
char* jconfig_group_name( JConfig* conf, int index )
{
  JConfigGroup* cgroup;

  if( !conf ) return (char*) 0L;

  for( cgroup = conf->groups; cgroup; cgroup = cgroup->next )
    if( index-- <= 0 ) break;

  return cgroup ? cgroup->name : (char*) 0L;
}

/*==================================================================*/
/* internal: find a specific group, NULL if not found */
JConfigGroup* jconfig_find_group( JConfig* conf, char* group_name )
{
  JConfigGroup* group;
  char *name;

  if( !conf ) return (JConfigGroup*) 0L;

  name = jconfig_strip_spaces( group_name );
  for( group = conf->groups; group; group = group->next )
    if( !strcmp( name, group->name ) ||
       ( strlen(name)==0 && strlen(group->name)==0 ) ) break;

  jfree( name );

  return group;
}

/*==================================================================*/
int jconfig_has_group( JConfig* conf, char* group_name )
{
  JConfigGroup* group;
  group = jconfig_find_group( conf, group_name );
  return group != 0;
}

/*==================================================================*/
void jconfig_add_group( JConfig* conf, char* group_name )
{
  JConfigGroup* group;

  if( !conf ) return;

#ifdef DEBUG
  printf( "jconfig_add_group [%s]\n", group_name );
#endif

  if( jconfig_has_group( conf, group_name ) )
    return;

  group = (JConfigGroup*) jmalloc( sizeof( JConfigGroup ) );
  group->name = jconfig_strip_spaces( group_name );
  group->next = conf->groups;
  group->vars = 0;
  conf->groups = group;
}

/*==================================================================*/
void jconfig_remove_group( JConfig* conf, char* group_name )
{
  JConfigGroup* group;
  JConfigGroup* ptr;
  char *name;
  JConfigVar* var;

  if( !conf ) return;
  if( !conf->groups ) return;

#ifdef DEBUG
  printf( "jconfig_remove_group [%s]\n", group_name );
#endif

  name = jconfig_strip_spaces( group_name );

  group = jconfig_find_group( conf, name );
  if( group )
  {

    /* remove all variables belong to this group */
    var = group->vars;
    while( var )
    {
      jconfig_remove_var( conf, group->name, var->name );
      var = group->vars;
    }

    /* remove group from the list */
    group = ptr = 0;
    if( !strcmp( name, conf->groups->name ) )
    {
      group = conf->groups;
      conf->groups = conf->groups->next;
    }
    else
    for( ptr = conf->groups; ptr->next; ptr = ptr->next )
    {
      if( !strcmp( name, ptr->next->name) )
      {
         group = ptr->next;
         ptr->next = group->next;
         break;
      }
    }

    jfree( name );

    if( group )
    {
      jfree( group->name );
      jfree( group );
    }

  }
}

/*==================================================================*/
int jconfig_var_count( JConfig* conf, char* group_name )
{
  int count = 0;
  JConfigGroup* group;
  JConfigVar* var;

  if( !conf ) return 0;

  group = jconfig_find_group( conf, group_name );
  if( !group ) return 0;

  for( var = group->vars; var; var = var->next )
    count++;

  return count;
}

/*==================================================================*/
char* jconfig_var_name( JConfig* conf, char* group_name, int index )
{
  JConfigGroup* group;
  JConfigVar* var;

  if( !conf ) return (char*) 0L;

  group = jconfig_find_group( conf, group_name );
  if( !group ) return 0;

  for( var = group->vars; var; var = var->next )
    if( index-- <= 0 ) break;

  return var ? var->name : (char*) 0L;
}


/*==================================================================*/
/* internal: find a specific variable */
JConfigVar* jconfig_find_var( JConfig* conf, char* group_name, char* var_name )
{
  JConfigGroup* group;
  JConfigVar* var;
  char* name;

  if( !conf ) return 0;

  group = jconfig_find_group( conf, group_name );
  if( !group ) return 0;

  name = jconfig_strip_spaces( var_name );
  for( var = group->vars; var; var = var->next )
    if( !strcmp( name, var->name ) ) break;
  jfree( name );

  return var;
}

/*==================================================================*/
int jconfig_has_var( JConfig* conf, char* group_name, char* var_name )
{
  JConfigVar* var;
  var = jconfig_find_var( conf, group_name, var_name );
  return var != 0;
}

/*==================================================================*/
void jconfig_add_var( JConfig* conf, char* group_name, char* var_name )
{
  JConfigGroup* group;
  JConfigVar* var;

  if( !conf ) return;

  if( jconfig_has_var( conf, group_name, var_name ) )
    return;

#ifdef DEBUG
  printf( "jconfig_add_var [%s,%s]\n", group_name, var_name );
#endif

  if( !jconfig_has_group( conf, group_name ) )
    jconfig_add_group( conf, group_name );

  group = jconfig_find_group( conf, group_name );
  if( !group ) return;

  var = (JConfigVar*) jmalloc( sizeof( JConfigVar ) );
  var->name = jconfig_strip_spaces( var_name );
  var->value = jstrdup("");
  var->next = group->vars;
  group->vars = var;
}

/*==================================================================*/
void jconfig_remove_var( JConfig* conf, char* group_name, char* var_name )
{
  JConfigGroup *group;
  JConfigVar *var;
  JConfigVar *ptr;
  char* name;

  if( !conf ) return;

#ifdef DEBUG
  printf( "jconfig_remove_var [%s,%s]\n", group_name, var_name );
#endif

  group = jconfig_find_group( conf, group_name );
  if( !group ) return;
  if( !group->vars )  return;

  name = jconfig_strip_spaces( var_name );

  var = ptr = 0;
  if( !strcmp( name, group->vars->name ) )
  {
    var = group->vars;
    group->vars = group->vars->next;
  }
  else
  for( ptr = group->vars; ptr->next; ptr = ptr->next )
  {
    if( !strcmp( name, ptr->next->name) )
    {
      var = ptr->next;
      ptr->next = var->next;
      break;
    }
  }

  jfree( name );

  if( var )
  {
    jfree( var->name );
    jfree( var->value );
    jfree( var );
  }
}


/*==================================================================*/
int jconfig_read_int( JConfig* conf, char* group_name, char* var_name, int def )
{
  JConfigVar* var;
  
  if( !conf ) return def;

  var = jconfig_find_var( conf, group_name, var_name );
  if( !var ) return def;
  return atol( var->value );
}

/*==================================================================*/
char* jconfig_read_string( JConfig* conf, char* group_name, char* var_name, char* def )
{
  JConfigVar* var;

  if( !conf ) return (char*) 0L;

  var = jconfig_find_var( conf, group_name, var_name );
  return var ? var->value : def;
}

/*==================================================================*/
void jconfig_write_int( JConfig* conf, char* group_name, char* var_name, int value )
{
  char buf[100]; /* FIXME is it safe */

  if( !conf ) return;

  snprintf( buf, 99, "%d", value );
  jconfig_write_string( conf, group_name, var_name, buf );
}

/*==================================================================*/
void jconfig_write_string( JConfig* conf, char* group_name, char* var_name, char* value )
{
  JConfigVar* var;

  if( !conf ) return;

#ifdef DEBUG
  printf( "jconfig_write_string [%s,%s] to %s\n", group_name, var_name, value );
#endif

  jconfig_add_var( conf, group_name, var_name );

  var = jconfig_find_var( conf, group_name, var_name );
  if( !var ) return; /* shouldn't happen though */

  jfree( var->value );
  var->value = jconfig_strip_spaces( value ? value : " " );
}

void jconfig_dump( JConfig* conf )
{
  JConfigGroup* group;
  JConfigVar* var;

  if( !conf ) return;

  printf( "------------------\n");
  printf( "Config: %s\n", conf->filename );

  for( group = conf->groups; group; group = group->next )
  {
    printf( "[%s]\n", group->name );
    for( var = group->vars; var; var = var->next )
      printf( " %s : %s\n", var->name, var->value );
  }
  printf( "------------------\n");
}


/* Strips whitespaces from a string, returns a new a string */
/* TODO should we use isspace() from ctype.h ? */
char* jconfig_strip_spaces( char* string )
{
  char *result, *p;

  if( !string ) return (char*) 0L;

  /* just allocate big enough space */
  result = jstrdup( string );

  /* skip spaces at beginning */
  for( p = string; *p ; p++ )
  if( ( *p != 32 ) && ( *p != 9 ) ) break;

  strcpy( result, p );

  /* remove trailing spaces */
  if( strlen( result ) > 0 )
  {
    p = result + strlen( result ) - 1;
    while( (p >= result ) && ( *p==32 || *p==9 ) ) *p--= '\0';
  }

  return result;
}
