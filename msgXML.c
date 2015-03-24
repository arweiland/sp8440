/** 
 *  @file   msgXML.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Phone line registration parser
 * 
 * @section Description
 * Parses a line registration / line unregistration message from SP8440 phone
 *
 */


#include <stdio.h>
#include <string.h>
#include <expat.h>

#include "msgXML.h"

#define XML_FMT_INT_MOD "l"

enum
{
   EXPAT_ELEMENT,
   EXPAT_DATA,
   EXPAT_ENDELEMENT
};

char *expat_types[] = 
{
   "StartElement",
   "Data",
   "EndElement"
};


phone_reg_t phone_reg;

typedef struct
{
   char *elementName;
   char *data;
   int  len;
} expat_data_t;


expat_data_t expat_data[] = 
{
   {"PhoneIP", phone_reg.phoneIP, sizeof( phone_reg.phoneIP ) },
   {"MACAddress", phone_reg.MACAddress, sizeof( phone_reg.MACAddress ) },
   {"LineNumber", phone_reg.LineNumber, sizeof( phone_reg.LineNumber ) },
   {"TimeStamp", phone_reg.TimeStamp, sizeof( phone_reg.TimeStamp ) }
};


#define N_EXPAT_DATA  (sizeof( expat_data ) / sizeof( expat_data_t ) )


static int expat_depth = 0;
static int expat_OK = 0;

phone_reg_t *msgXML_parseRegistration( char *buf, int len );
static void _msgXML_processReg( int type, const char *data );
static void _msgXML_saveData( const char *el, const char *data );

static void XMLCALL _startElement(void *userData, const char *name, const char **atts);
static void XMLCALL _endElement(void *userData, const char *name);
static void XMLCALL _charParse(void *userData, const XML_Char *s, int len);

#if 0
int main( int argc, char *argv[] )
{
   phone_reg_t *phone_reg_data;
   char buf[BUFSIZ];

   int len = (int)fread(buf, 1, sizeof(buf), stdin);

   if ( (phone_reg_data = msgXML_parseRegistration( buf, len )) == NULL )
   {
      printf( "Parse failed!\n" );
      return 1;
   }
   printf( "Phone IP: %s\n", phone_reg.phoneIP );
   printf( "Mac address: %s\n", phone_reg.MACAddress );
   printf( "Line Number: %s\n", phone_reg.LineNumber );
   printf( "Time Stamp: %s\n", phone_reg.TimeStamp );
   return 0;
}
#endif

phone_reg_t *msgXML_parseRegistration( char *msg, int len )
{

  expat_depth = 0;         // start no depth
  expat_OK = 0;            // wait for next OK to process data

  XML_Parser parser = XML_ParserCreate(NULL);

  XML_SetElementHandler(parser, _startElement, _endElement);
  XML_SetCharacterDataHandler( parser, _charParse );

  if (XML_Parse(parser, msg, len, 1) == XML_STATUS_ERROR)
  {
     fprintf(stderr,
             "%s at line %" XML_FMT_INT_MOD "u\n",
             XML_ErrorString(XML_GetErrorCode(parser)),
             XML_GetCurrentLineNumber(parser));
     return NULL;
  }

  XML_ParserFree(parser);

  return &phone_reg;
}


static void _msgXML_processReg( int type, const char *data )
{
   static const char *last_el;

   if ( type == EXPAT_ELEMENT )
   {
      expat_depth++;
      last_el = data;
      if ( expat_depth == 2 )
      {
         if ( strcasestr( last_el, "registration" ) != NULL )
         {
            expat_OK = 1;       // process this message
         }
      }
   }

   else if (type == EXPAT_DATA )
   {
      if ( expat_depth == 3 && expat_OK )
      {
         _msgXML_saveData( last_el, data );
      }
   }
   else if (type == EXPAT_ENDELEMENT )
   {
      expat_depth--;
      if ( expat_depth == 0 )
      {
         expat_OK = 0;      // wait till next message
      }
   }
}


static void _msgXML_saveData( const char *el, const char *data )
{
   expat_data_t *eptr;
   int found = 0;
   int i;

   for ( i = 0, eptr = expat_data; i < N_EXPAT_DATA; i++, eptr++ )
   {
      if ( strcasestr( el, eptr->elementName ) != NULL )
      {
         strncpy( eptr->data, data, eptr->len );
         eptr->data[ eptr->len-1 ] = '\0';
         found = 1;
//         printf( "Saved data element %s: %s\n", el, data );
      }
   }

   if ( !found )
   {
      printf( "Element %s not found\n", el );
   }
}



// Called when element starts.  Depth pointer is passed in
static void XMLCALL _startElement(void *userData, const char *name, const char **atts)
{
  _msgXML_processReg( EXPAT_ELEMENT, name );
}

// Called when element ends.  Just decrements depth pointer
static void XMLCALL _endElement(void *userData, const char *name)
{
  _msgXML_processReg( EXPAT_ENDELEMENT, NULL );
}

// Data within elements
static void XMLCALL _charParse(void *userData, const XML_Char *s, int len)
{
   char buf[ 100 ];
   if (len < 100)
   {
      memcpy( buf, s, len );
      buf[len] = '\0';
   }
   _msgXML_processReg( EXPAT_DATA, buf );
}

