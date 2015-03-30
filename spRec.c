/** 
 *  @file   spRec.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Handles phone status
 *
 *  @section Description 
 * 
 * Contains code to track the status of phones.\n
 * Saves current status in JSON in case of reboot.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "spRec.h"
#include "cJSON.h"
#include "config.h"
#include "logging.h"

int max_SPphones;                                // max phones allowed (from config)

SPphone_record_t *SPphones;                      // array of Spectralink phone records

int _spRec_ParseFile( void );                    // Read JSON file and parse
void _spRec_ParsePhones( char *data );           // read phone records from JSON
void _spRec_EncodePhones( void );                // write phone records to JSON

// Helper functions
void _spRec_GetStr( cJSON *root, char *what, char *dest, int len );
void _spRec_GetInt( cJSON *root, char *what, int *dest );

static char *outfile;                   // output file name

#if 0

int main( void )
{
   SPphone_record_t *sptr = NULL;

   spRec_Init();
   _spRec_EncodePhones();               // Try encoding current phone data

   while( (sptr = spRec_GetNextRecord( sptr )) != NULL )
   {
      printf( "IP address: %s\n", sptr->ip_addr );
   }

   spRec_AddRecord( "192.168.1.101", "11-22-33-44-11-22" );
//   spRec_RemoveIP( "192.168.1.11" );
   return 0;
}

#endif


/*-----------------( spRec_Init )----------------------------

-----------------------------------------------------------*/

int spRec_Init( void )
{   
   int memsize;

   outfile = config_readStr( "phones", "records_file", "phones.json" );

   // Get max number of phones from config, default to 50
   max_SPphones = config_readInt( "phones", "max_phones", 50 );

   memsize = (max_SPphones + 1) * sizeof( SPphone_record_t );

   SPphones = (SPphone_record_t *)malloc( memsize );
   SPphones[ max_SPphones ].in_use = -1;     // sentinal node (end of array)
   Log( INFO, "%s Created room for %d phones\n", __func__, max_SPphones );
   _spRec_ParseFile();

   return 0;
}


/*---------------------( spRec_CheckStale )-----------------------

  Check for any phones not seen in a while.  Remove them.

----------------------------------------------------------------*/

void spRec_CheckStale( void )
{
   SPphone_record_t *sptr = NULL;
   time_t max_time;
   time_t curtime;
   int dirty = 0;

   // get max last seen time from config
   max_time = config_readInt( "phones", "max_last_seen", 60 );
   curtime = time(NULL);         // get current time

   while ( (sptr = spRec_GetNextRecord( sptr )) != NULL )
   {
      if ( (curtime - sptr->last_seen) > (max_time * 60) )
      {
         sptr->in_use = 0;       // remove this one
         dirty = 1;              // database changed
         PLog( INFO, "Removing %s, Not seen in %ld minutes\n", sptr->ip_addr, (long)max_time );
         Log( DEBUG, "%s: Removing stale phone: %s. Not seen in %ld minutes\n", __func__, sptr->ip_addr, (long)max_time );
      }
   }

   if ( dirty )
   {
      _spRec_EncodePhones();    // write new file
   }
}



/*--------------------( spRec_AddRecord )----------------------

  Add a new phone record.
  If there is already a record with same IP but different MAC, 
  or same MAC but different IP, remove it and replace with this one.
  If this phone is already in record, just update last seen time.

  Return 0 if OK, -1 if array full
-----------------------------------------------------------*/

int spRec_AddRecord( char *ip_addr, char *mac, int line_num )
{
   SPphone_record_t *sptr = NULL;
   int same_mac;
   int same_ip;

   // check if there is an existing record with same IP but different MAC,
   // or same MAC but different IP.
   while( (sptr = spRec_GetNextRecord( sptr )) != NULL )
   {
      same_mac = !strcmp( sptr->mac, mac );
      same_ip =  !strcmp( sptr->ip_addr, ip_addr );
      if ( (same_mac && !same_ip) || (same_ip && !same_mac) )
      {
         sptr->in_use = 0;                            // remove existing
         Log( DEBUG, "%s: Removing old record for %s\n", __func__, ip_addr );
      }
   }

   // If still record with same IP, it is this phone. Update last seen time
   if ( (sptr = spRec_FindIP( ip_addr )) != NULL )
   {
      sptr->last_seen = time( NULL );                 // Record current time
      Log( DEBUG, "%s: %s already in record, updating last_seen\n", __func__, ip_addr );
   }

   // If new record, add it
   else
   {
      sptr = SPphones;
      while ( (sptr->in_use) != 0 && (sptr->in_use != -1) )
      {
         sptr++;
      }

      if ( sptr->in_use == -1 )
      {
         Log( WARN, "%s: Phone records are full!\n", __func__ );
         return -1;
      }

      strncpy( sptr->ip_addr, ip_addr, sizeof( sptr->ip_addr ) );
      sptr->ip_addr[ sizeof( sptr->ip_addr )-1 ] = '\0';   // make sure it is terminated

      strncpy( sptr->mac, mac, sizeof( sptr->mac ) );
      sptr->mac[ sizeof( sptr->mac )-1 ] = '\0';           // make sure it is terminated

      sptr->line_number = line_num;                // line number
      sptr->last_seen = time( NULL );              // Record current time
      sptr->in_use = 1;                            // now in use
      Log( INFO, "%s: Created new record for %s\n", __func__, ip_addr );
      PLog( INFO, "Created new record for %s, line %d\n", ip_addr, line_num );
   }

   _spRec_EncodePhones();                          // Write new file

   return 0;
}


/*---------------( spRec_RemoveIP )-------------------

  Remove a phone based on its IP address.
  Write out new JSON file

---------------------------------------------------*/

void spRec_RemoveIP( char *ip_addr )
{
   SPphone_record_t *sptr;

   if ( (sptr = spRec_FindIP( ip_addr )) != NULL )
   {
      sptr->in_use = 0;                  // Remove record
      _spRec_EncodePhones();             // Write new file
   }
}


/*---------------( spRec_FindIP )-------------------

  Find a phone record based on its IP address.

  Return pointer to record, or NULL if not found
---------------------------------------------------*/

SPphone_record_t *spRec_FindIP( char *ip_addr )
{
   SPphone_record_t *sptr = NULL;

   while( (sptr = spRec_GetNextRecord( sptr )) != NULL )
   {
      if ( strcmp( sptr->ip_addr, ip_addr ) == 0 )
      {
         return sptr;
      }
   }
   return NULL;
}


/*---------------( spRec_GetNextRecord )----------------

  Return a pointer to the next phone record.
  Returns NULL if no more records.

  If sptr = NULL, start at beginning.
-----------------------------------------------------*/

SPphone_record_t *spRec_GetNextRecord( SPphone_record_t *sptr )
{
   SPphone_record_t *SPptr;

   if ( sptr == NULL )
   {
      SPptr = SPphones;
   }
   else if ( sptr->in_use == -1 )      // already at last?
   {
      return NULL;
   }
   else
   {
      SPptr = sptr+1;
   }

   while ( SPptr->in_use != -1 )
   {
      if (SPptr->in_use == 1 )
      {
         return SPptr;
      }
      else
      {
         SPptr++;
      }
   }
   return NULL;
}

/*---------------( _spRec_EncodePhones )---------------

  Write out current phones structure in JSON format

-----------------------------------------------------*/

void _spRec_EncodePhones(void)
{
   FILE *fptr;
   SPphone_record_t *SPptr;

   if ( (fptr = fopen( outfile, "w" )) == NULL )
   {
      Log( ERROR, "%s: Can't open file \"%s\" for writing: %s\n", __func__, outfile, strerror(errno) );
      return;
   }

   fprintf( fptr, "{\"phones\": [\n" );
   SPptr = NULL;
   while ( (SPptr = spRec_GetNextRecord( SPptr )) != NULL )
   {
      fprintf( fptr, "\t{\n" );
      fprintf( fptr, "\t\t\"mac\" : \"%s\",\n", SPptr->mac );
      fprintf( fptr, "\t\t\"ip_addr\" : \"%s\",\n", SPptr->ip_addr );
      fprintf( fptr, "\t\t\"line_number\" : %d,\n", SPptr->line_number );
      fprintf( fptr, "\t\t\"last_seen\" : %d\n", SPptr->last_seen );
      fprintf( fptr, "\t}" );
      if ( spRec_GetNextRecord( SPptr ) != NULL )       // another record coming?
      {
         fprintf( fptr, "," );                 // Add comma separator (required)
      }
      fprintf( fptr, "\n" );
   }

   fprintf( fptr, "]}\n");
   fclose( fptr );
   Log( DEBUG, "%s. Wrote records file\n", __func__ );
}

/*-----------------( _spRec_ParseFile )-------------------

  Read the JSON file and parse it

  Returns 0 if everything good, else -1 if error
-----------------------------------------------------*/

int _spRec_ParseFile( void )
{
   FILE *fptr;
   long fsize;
   char *buffer;

   if ( (fptr = fopen( outfile, "r" )) == NULL )
   {
      Log( WARN, "%s: Can't open file for reading \"%s\"\n", __func__, outfile );
      return -1;
   }

   // Get file size
   fseek( fptr, 0, SEEK_END);
   fsize = ftell(fptr);      // get size of file
   fseek(fptr, 0, SEEK_SET);

   if ( (buffer = (char *)malloc( fsize )) == NULL )
   {
      Log( ERROR, "%s: Can't malloc buffer!\n", __func__ );
      fclose( fptr );
      return -1;
   }

   fread( (void *)buffer, 1, fsize, fptr );

   // Parse the file
   _spRec_ParsePhones( buffer );

   fclose( fptr );
   free( buffer );
   return 0;
}


/*---------------( _spRec_ParsePhones )------------------

  Parse phone data from file buffer

-----------------------------------------------------*/

void _spRec_ParsePhones( char *data )
{
   cJSON *request_body;          // top body
   int i, nRecords;
   SPphone_record_t *SPptr;

   request_body = cJSON_Parse( data );

   cJSON *item = cJSON_GetObjectItem( request_body, "phones" );
   if ( item == NULL )
   {
      Log( ERROR, "%s: Can't find phones in %s!\n", __func__, outfile );
      return;
   }

   if ( (nRecords = cJSON_GetArraySize(item) ) > max_SPphones )
   {
      nRecords = max_SPphones;         // limit to size of records
   }

   Log( DEBUG, "%s: %d phone records found\n", __func__, nRecords);

   for (i = 0, SPptr = SPphones ; i < nRecords ; i++, SPptr++)
   {
      cJSON * subitem = cJSON_GetArrayItem(item, i);
      _spRec_GetStr( subitem, "MAC", SPptr->mac, sizeof( SPptr->mac ) );
      _spRec_GetStr( subitem, "ip_addr", SPptr->ip_addr, sizeof(SPptr->ip_addr) );
      _spRec_GetInt( subitem, "line_number", &(SPptr->line_number) );
      _spRec_GetInt( subitem, "last_seen", &(SPptr->last_seen) );
      Log( DEBUG, "%s: phone %d. MAC: %s,  ip_addr: %s, line_number: %d, last_seen: %d\n", 
           __func__, i+1, SPptr->mac, SPptr->ip_addr, SPptr->line_number, SPptr->last_seen );
      SPptr->in_use = 1;
   }
   cJSON_Delete( request_body );
}

void _spRec_GetStr( cJSON *root, char *what, char *dest, int len )
{
   cJSON *item = cJSON_GetObjectItem(root, what);

   if (item == NULL)     // item not found?
   {
      return;
   }
   strncpy( dest, item->valuestring, len );
   dest[ len-1] = '\0';     // Make sure it is null-terminated
}

void _spRec_GetInt( cJSON *root, char *what, int *dest )
{
   cJSON *item = cJSON_GetObjectItem(root, what);

   if (item == NULL)     // item not found?
   {
      return;
   }
   *dest = item->valueint;
}

