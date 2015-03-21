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

#define MAX_SPPHONES 50            // max phones allowed (should come from config file)

SPphone_record_t *SPphones;                      // array of Spectralink phone records

int spRec_ParseFile( char *fpath );              // Read JSON file and parse
void spRec_ParsePhones( char *data );            // read phone records from JSON
void spRec_EncodePhones( char *fpath );          // write phone records to JSON

// Helper functions
void spRec_GetStr( cJSON *root, char *what, char *dest, int len );
void spRec_GetInt( cJSON *root, char *what, int *dest );

char *fname = "phones.json";
char *outname = "phones_new.json";       // temporary output file name

#ifdef TEST

int main( void )
{
   SPphone_record_t *sptr = NULL;

   spRec_Init();
   spRec_EncodePhones( outname );        // Try encoding current phone data

   while( (sptr = spRec_GetNextRecord( sptr )) != NULL )
   {
      printf( "IP address: %s\n", sptr->ip_addr );
   }

   spRec_AddRecord( "192.168.1.101", "11-22-33-44-11-22" );
//   spRec_RemoveIP( "192.168.1.11" );
   return 0;
}

#endif


/*-----------------( spRec_Init )-------------------

-------------------------------------------------*/

void spRec_Init( void )
{
   SPphones = (SPphone_record_t *)malloc( (MAX_SPPHONES + 1) * sizeof( SPphone_record_t ) );
   SPphones[ MAX_SPPHONES ].in_use = -1;
   spRec_ParseFile( fname );
}

/*---------------( spRec_AddRecord )-------------------

  Add a new phone record

  Return 0 if OK, -1 if array full
---------------------------------------------------*/

int spRec_AddRecord( char *ip_addr, char *mac )
{
   SPphone_record_t *sptr = SPphones;

   if ( spRec_FindIP( ip_addr ) != NULL )
   {
      return 0;         // already in record
   }

   // find record not in use
   while( sptr->in_use == 1 )
   {
      sptr++;
   }

   if (sptr->in_use == -1 )           // past end?
   {
      printf( "Phones array full!\n" );
      return -1;
   }   

   strncpy( sptr->ip_addr, ip_addr, sizeof( sptr->ip_addr ) );
   sptr->ip_addr[ sizeof( sptr->ip_addr )-1 ] = '\0';   // make sure it is terminated
   strncpy( sptr->mac, mac, sizeof( sptr->mac ) );
   sptr->mac[ sizeof( sptr->mac )-1 ] = '\0';   // make sure it is terminated

   sptr->last_seen = time( NULL );              // Record current time
   sptr->in_use = 1;                            // now in use

   spRec_EncodePhones( outname );               // Write new file


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
      spRec_EncodePhones( outname );     // Write new file
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

/*---------------( spRec_EncodePhones )------------------

  Write out current phones structure in JSON format

-----------------------------------------------------*/

void spRec_EncodePhones( char *fpath )
{
   FILE *fptr;
   SPphone_record_t *SPptr;

   if ( (fptr = fopen( fpath, "w" )) == NULL )
   {
      printf( "Can't open file \"%s\" for writing: %s\n", fpath, strerror(errno) );
      return;
   }

   fprintf( fptr, "{\"phones\": [\n" );
   SPptr = NULL;
   while ( (SPptr = spRec_GetNextRecord( SPptr )) != NULL )
   {
      fprintf( fptr, "\t{\n" );
      fprintf( fptr, "\t\t\"mac\" : \"%s\",\n", SPptr->mac );
      fprintf( fptr, "\t\t\"ip_addr\" : \"%s\",\n", SPptr->ip_addr );
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
}

/*-----------------( spRec_ParseFile )-------------------

  Read the JSON file and parse it

  Returns 0 if everything good, else -1 if error
-----------------------------------------------------*/

int spRec_ParseFile( char *fpath )
{
   FILE *fptr;
   long fsize;
   char *buffer;

   if ( (fptr = fopen( fpath, "r" )) == NULL )
   {
      printf( "Can't open file \"%s\"\n", fpath );
      return -1;
   }

   // Get file size
   fseek( fptr, 0, SEEK_END);
   fsize = ftell(fptr);      // get size of file
   fseek(fptr, 0, SEEK_SET);

   if ( (buffer = (char *)malloc( fsize )) == NULL )
   {
      printf( "spRecInit: Can't malloc buffer!\n" );
      fclose( fptr );
      return -1;
   }

   fread( (void *)buffer, 1, fsize, fptr );

   // Parse the file
   spRec_ParsePhones( buffer );

   fclose( fptr );
   free( buffer );
   return 0;
}


/*---------------( spRec_ParsePhones )------------------

  Parse phone data from file buffer

-----------------------------------------------------*/

void spRec_ParsePhones( char *data )
{
   cJSON *request_body;          // top body
   int i, nRecords;
   SPphone_record_t *SPptr;

   request_body = cJSON_Parse( data );

   cJSON *item = cJSON_GetObjectItem( request_body,"phones");
   if ( item == NULL )
   {
      printf( "Can't find phones\n" );
      return;
   }

   nRecords = cJSON_GetArraySize(item); 
   printf( "%d phone records found\n", nRecords);

   for (i = 0, SPptr = SPphones ; i < nRecords ; i++, SPptr++)
   {
      cJSON * subitem = cJSON_GetArrayItem(item, i);
      spRec_GetStr( subitem, "MAC", SPptr->mac, sizeof( SPptr->mac ) );
      spRec_GetStr( subitem, "ip_addr", SPptr->ip_addr, sizeof(SPptr->ip_addr) );
      spRec_GetInt( subitem, "last_seen", &(SPptr->last_seen) );
      printf( "phone %d. MAC: %s,  ip_addr: %s, last_seen: %d\n", i+1, SPptr->mac, SPptr->ip_addr, SPptr->last_seen );
      SPptr->in_use = 1;
   }
   cJSON_Delete( request_body );
}

void spRec_GetStr( cJSON *root, char *what, char *dest, int len )
{
   cJSON *item = cJSON_GetObjectItem(root, what);

   if (item == NULL)     // item not found?
   {
      return;
   }
   strncpy( dest, item->valuestring, len );
   dest[ len-1] = '\0';     // Make sure it is null-terminated
}

void spRec_GetInt( cJSON *root, char *what, int *dest )
{
   cJSON *item = cJSON_GetObjectItem(root, what);

   if (item == NULL)     // item not found?
   {
      return;
   }
   *dest = item->valueint;
}

