
#include <curl/curl.h>
#include <pthread.h>

#include "msgBuild.h"

#define MAX_DATA 2000

char *ip_addrs[] = 
{
   "192.168.1.248",
   "192.168.2.2",
   "192.168.2.3",
   "192.168.2.4",
   "192.168.1.221",
};

#define N_IP_ADDRS (sizeof( ip_addrs ) / sizeof( char *))

/*---  structure to pass to thread ---*/
typedef struct
{
   char *ip_addr;
   char *msg;
}curlThreadMsg_t;


void *msgSend_PushMsgThread( void *ip_addr );
size_t msgSend_WriteCallback( void *buffer, size_t size, size_t nmemb, void *data );

#ifdef TEST

int main( void )
{
   int i;
   pthread_t tid[ N_IP_ADDRS ];              // array of thread ids
   char msg_buf[ MAX_DATA ];                 // message buffer to send
   curlThreadMsg_t msgs[ N_IP_ADDRS ];       // array of message data

   // create the message to send
   msgBuild_makePushMsg( "message.html", msg_buf, MAX_DATA, 100, "Tools" );

   // start the threads to send the data
   for( i = 0; i < N_IP_ADDRS; i++ )
   {
      msgs[i].msg = msg_buf;                // pointer to message to send
      msgs[i].ip_addr = ip_addrs[i];        // pointer to IP address to send to
      pthread_create( &tid[i], NULL, msgSend_PushMsgThread, (void *)&msgs[i] );
   }

   /* now wait for all threads to terminate */
   for(i=0; i< N_IP_ADDRS; i++)
   {
      pthread_join(tid[i], NULL);
      fprintf(stderr, "Thread %d terminated\n", i);
   }

   return 0;
}
#endif

void *msgSend_PushMsgThread( void *msg )
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

   curl_easy_setopt(hnd, CURLOPT_USERPWD, "admin:456");
   curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, msgData->msg);
   curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3");
   curl_easy_setopt(hnd, CURLOPT_HTTPAUTH, 2L);

   curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 5L );                        // Set timeout to 5 seconds
   curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, msgSend_WriteCallback );   // Set the received data callback function
   curl_easy_setopt(hnd, CURLOPT_WRITEDATA, msg );                     // Structure to send to callback function

   /*--- Unneeded operations ---*/
   // curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, errorBuf );  // Set error buffer
   // curl_easy_setopt(hnd, CURLOPT_SSH_KNOWNHOSTS, "/home/rweiland/.ssh/known_hosts");
   // curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
   // curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)-1);
   // curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);

   printf( "Sending to %s\n", msgData->ip_addr );

   if ( (ret = curl_easy_perform(hnd)) != 0 )
   {
      printf( "Failed on connection \"%s\": error: %d, %s\n", msgData->ip_addr, ret, curl_easy_strerror(ret) );
   }
   else
   {
      curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpCode );       // Get the HTTP response code
      if ( httpCode == 200 )
      {
         printf( "Sent successfully to \"%s\"\n", msgData->ip_addr );
      }
      else
      {
         printf( "\nSend Failed on \"%s\". Response code: %ld\n\n", msgData->ip_addr, httpCode );
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

/*--------------------------( msgSender_WriteCallback )----------------------------
  This function receives data from the phones in response to the data push.
  We don't want to do anything with it, we just don't want it to go to stdout.
  Data pointed to by buffer is NOT null-terminated!

  This function MUST return the total number of bytes passed to it!
-----------------------------------------------------------------------------*/

size_t msgSend_WriteCallback( void *buffer, size_t size, size_t nmemb, void *data )
{
   printf( "WriteCallback called for address %s\n", ((curlThreadMsg_t *)data)->ip_addr );

//   printf( "Size: %d:%d, Data: \n", (int)size, (int)nmemb );
//   fwrite( buffer, size, nmemb, stdout );

   return size * nmemb;         // Tell curl we've handled the data
}

