/** 
 *  @file   msgSend.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Message sender
 * 
 * @section Description
 * Creates and sends the HTML messages to the phones in parallel using libcurl
 *
 */

#include <curl/curl.h>
#include <pthread.h>
#include <string.h>
#include <malloc.h>

#include "msgSend.h"
#include "msgBuild.h"
#include "spRec.h"
#include "config.h"
#include "logging.h"
#include "alarms.h"

#define MAX_HTML_DATA 2000       // max size of HTML message to send

/*
char *ip_addrs[] = 
{
   "192.168.1.248",
   "192.168.2.2",
   "192.168.2.3",
   "192.168.2.4",
   "192.168.1.221",
};

#define N_IP_ADDRS (sizeof( ip_addrs ) / sizeof( char *))
*/

#define MAX_SPPHONES    50            // max phones to send to

/*---  structure to pass to thread ---*/
typedef struct
{
   char *ip_addr;
   char *msg;
}curlThreadMsg_t;

curlThreadMsg_t msgs[ MAX_SPPHONES ];         // array of message data

char alert_msgBuf[ MAX_HTML_DATA ];           // Alert message buffer to send
char accept_msgBuf[ MAX_HTML_DATA ];          // Accept message buffer to send
char accept_msgBuf2[ MAX_HTML_DATA ];         // Special Accept message buffer to send

static char *alert_template;                  // alert template file name / path
static char *accept_template;                 // accept template file name / path
static int net_timeout = 0;                   // How long to wait for response from phone
static char authentication[40];               // username / password to send for authentication

int _msgSend_PushMsgs( char *msg, char *special_ip, char *specal_msg );
void *_msgSend_PushMsgThread( void *ip_addr );
size_t _msgSend_WriteCallback( void *buffer, size_t size, size_t nmemb, void *data );

#if 0
int main( void )
{
   spRec_Init();                              // initialize phone records handler
   msgSend_PushMsgs( "tools" );
   return 0;
}
#endif

void msgSend_PushAlert( char *dept, int alarm, int level )
{
   char fname[100];

   if ( alert_template == NULL );
   {
      sprintf( fname, "%s%s", BASEDIR, config_readStr( "phones", "alert_template", "alert" ));
      alert_template = malloc( strlen( fname )+1 );
      strcpy( alert_template, fname );               // copy over file name with path
   }

   // create the message to send
   msgBuild_makeAlertMsg( alert_template, alert_msgBuf, MAX_HTML_DATA, dept, alarm, level );

   if ( _msgSend_PushMsgs( alert_msgBuf, NULL, NULL ) == 0 )         // no phones available?
   {
      Log( INFO, "%s: No phones available.  Escalating alarm %d now\n", __func__, alarm );
      escalate_alarm( alarm );          // escalate alarm now
   }
}

void msgSend_PushAccept( char *dept, int type, char *accept_ip )
{
   char *msg;
   char fname[100];

   if ( accept_template == NULL )
   {
      sprintf( fname, "%s%s", BASEDIR, config_readStr( "phones", "accept_template", "accept" ));
      accept_template = malloc( strlen( fname )+1 );
      strcpy( accept_template, fname );               // copy over file name with path
   }

   msg = (type == 0) ? "Request Accepted" : "Request Complete";

   // Make accept message for all phones except the one that accepted
   msgBuild_makeAcceptMsg( accept_template, accept_msgBuf, MAX_HTML_DATA, dept, msg );

   // Make accept message for phone that accepted
   msgBuild_makeAcceptMsg( accept_template, accept_msgBuf2, MAX_HTML_DATA, dept, "You've accepted" );

   // Send to all phones
   _msgSend_PushMsgs( accept_msgBuf, accept_ip, accept_msgBuf2 );
}


int _msgSend_PushMsgs( char *msg, char *special_ip, char *special_msg )
{
   int i;
   pthread_t tid[ MAX_SPPHONES ];            // array of thread ids
   int msgIndex;
   SPphone_record_t *phone;                  // phone informatiion
   char *username;
   char *password;
   int len;

   // If we haven't read values from config yet, try now
   if ( net_timeout == 0 || *authentication == '\0' )     // Haven't read values yet?
   {
      net_timeout = config_readInt( "phones", "phone_timeout", 5 );
      Log( DEBUG, "%s: Setting phone timeout to %d\n", __func__, net_timeout ); 
      username = config_readStr( "phones", "phone_username", "admin" );
      password = config_readStr( "phones", "phone_password", "456" );
      len = snprintf( authentication, sizeof( authentication ), "%s:%s", username, password );      // authentication string
      if ( len >= sizeof( authentication ) )
      {
         Log(WARN, "%s: Authenticaton string too long! \"%s\". Max size is %d bytes\n", __func__, authentication, (int)sizeof(authentication));
      }
   }

   // create the send threads
   phone = NULL;                             // start with first record
   msgIndex = 0;
   while( (phone = spRec_GetNextRecord( phone )) != NULL )
   {
      msgs[ msgIndex ].ip_addr = phone->ip_addr;   // IP address of phone to send to

      // check if special message for this IP
      if ( (special_ip != NULL) && (strncmp( special_ip, phone->ip_addr, strlen( phone->ip_addr)) == 0 ))
      {
         msgs[ msgIndex ].msg = special_msg;       // use special message
      }
      else
      {
         msgs[ msgIndex ].msg = msg;               // message pointer
      }
      pthread_create( &tid[ msgIndex ], NULL, _msgSend_PushMsgThread, (void *)&msgs[ msgIndex ] );
      msgIndex++;
   }

#if 0
   /* now wait for all threads to terminate */
   for(i=0; i< msgIndex; i++)
   {
      pthread_join(tid[i], NULL);
//      fprintf(stderr, "Thread %d terminated\n", i);
   }
#else
   for(i=0; i< msgIndex; i++)
   {
      pthread_detach(tid[i]);
   }
#endif

   return msgIndex;            // return number of phones messages are being sent to
}


void *_msgSend_PushMsgThread( void *msg )
{
   int ret;
   curlThreadMsg_t *msgData;
   char ip_buf[ 40 ];
   long httpCode = 0L;

//   char errorBuf[ CURL_ERROR_SIZE ];       // CURL error buffer

   msgData = (curlThreadMsg_t *)msg;
   CURL *hnd = curl_easy_init();

   // Create OPT with given IP address
   sprintf( ip_buf, "http://%s/push", msgData->ip_addr );
   curl_easy_setopt(hnd, CURLOPT_URL, ip_buf );

//   curl_easy_setopt(hnd, CURLOPT_USERPWD, "admin:456");
   curl_easy_setopt(hnd, CURLOPT_USERPWD, authentication );
   curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, msgData->msg);
   curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3");
   curl_easy_setopt(hnd, CURLOPT_HTTPAUTH, 2L);

   curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 5L );                        // Set timeout to 5 seconds
   curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, _msgSend_WriteCallback );   // Set the received data callback function
   curl_easy_setopt(hnd, CURLOPT_WRITEDATA, msg );                     // Structure to send to callback function
   curl_easy_setopt(hnd, CURLOPT_NOSIGNAL, 1);                         // shut off signals (to avoid "Alarm clock" in pthreads)

   /*--- Unneeded operations ---*/
   // curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, errorBuf );  // Set error buffer
   // curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
   // curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)-1);
   // curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);

   Log( DEBUG, "%s: Sending to %s\n", __func__, msgData->ip_addr );

   if ( (ret = curl_easy_perform(hnd)) != 0 )
   {
      Log( DEBUG, "%s: Failed on connection \"%s\": error: %d, %s\n", __func__, msgData->ip_addr, ret, curl_easy_strerror(ret) );
   }
   else
   {
      curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpCode );       // Get the HTTP response code
      if ( httpCode == 200 )
      {
         PLog( INFO, "%s Msg push successful to \"%s\"\n", __func__, msgData->ip_addr );
      }
      else
      {
         PLog( WARN, "%s Send Failed on \"%s\". Response code: %ld\n\n", __func__, msgData->ip_addr, httpCode );
      }
   }

   curl_easy_cleanup(hnd);

   /* Here is a list of options the curl code used that cannot get generated
      as source easily. You may select to either not use them or implement
      them yourself.

      CURLOPT_WRITEDATA set to a objectpointer
      CURLOPT_WRITEFUNCTION set to a functionpointer
      CURLOPT_READDATA set to a objectpointer
      CURLOPT_READFUNCTION set to a functionpointer
      CURLOPT_SEEKDATA set to a objectpointer
      CURLOPT_SEEKFUNCTION set to a functionpointer
      CURLOPT_ERRORBUFFER set to a objectpointer
      CURLOPT_HTTPHEADER set to a objectpointer
      CURLOPT_STDERR set to a objectpointer
      CURLOPT_SOCKOPTFUNCTION set to a functionpointer
      CURLOPT_SOCKOPTDATA set to a objectpointer

   */
   return NULL;
}

/*-------------------------( _msgSender_WriteCallback )-------------------------
  This function receives data from the phones in response to the data push.
  We don't want to do anything with it, we just don't want it to go to stdout.
  Data pointed to by buffer is NOT null-terminated!

  This function MUST return the total number of bytes passed to it!
-----------------------------------------------------------------------------*/

size_t _msgSend_WriteCallback( void *buffer, size_t size, size_t nmemb, void *data )
{
//   printf( "%s called for address %s\n", __func__, ((curlThreadMsg_t *)data)->ip_addr );

//   printf( "Size: %d:%d, Data: \n", (int)size, (int)nmemb );
//   fwrite( buffer, size, nmemb, stdout );

   return size * nmemb;         // Tell curl we've handled the data
}

