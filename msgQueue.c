/** 
 *  @file   msgQueue.c
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/30/15
 *  @brief  Alarm Message queue
 * 
 * @section Description
 * Queues alarm messages from the system and sends them out at timed intervals
 *
 */

#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "msgQueue.h"
#include "queues.h"
#include "logging.h"
#include "msgSend.h"

#define MAX_DEPT_NAME 20

typedef struct 
{
   char dept[MAX_DEPT_NAME+1];    // department name
   int alarm;                     // alarm number
   int level;                     // escalation level
}msg_queue_t;

static QUEUE_ID msg_queue;

static int msgQueue_delay;       // Delay period between outputs

#define MAX_MSGS  10             // max alarms allowed in queue

static pthread_t msgQueue_tid;
static pthread_mutex_t msgQueue_mutex = PTHREAD_MUTEX_INITIALIZER;

void *_msgQueue_RunThread( void *msg );

void _msgQueue_Init( void )
{
   msg_queue = create_queue( sizeof( msg_queue_t ), MAX_MSGS );

   // Start up the run thread
   pthread_create( &msgQueue_tid, NULL, _msgQueue_RunThread, NULL );
}


void msgQueue_SetAccept( void )
{
   msgQueue_SetDelay( 2 );
}


void msgQueue_SetDelay( int delay )
{
   printf( "Setting delay to %d seconds\n", delay );
   pthread_mutex_lock( &msgQueue_mutex );
   msgQueue_delay = delay * 10;                 // internal delay period is 100ms
   pthread_mutex_unlock( &msgQueue_mutex );
}


int msgQueue_Add( char *msg, int alarm, int level )
{
   msg_queue_t qmsg;

   if ( msg_queue == (QUEUE_ID)0 )       // not initialized yet?
   {
      _msgQueue_Init();
   }

   strncpy( qmsg.dept, msg, MAX_DEPT_NAME );

   if ( strlen( msg ) > MAX_DEPT_NAME )
   {
      Log(WARN, "%s: Department name \"%s\" too long. Max = %d\n", __func__, msg, MAX_DEPT_NAME );
      qmsg.dept[MAX_DEPT_NAME] = '\0';
   }

   qmsg.alarm = alarm;             // alarm number
   qmsg.level = level;             // escalation level

   if ( enqueue_data( msg_queue, &qmsg, sizeof( qmsg )) == 0 )
   {
      Log( WARN, "%s: Msg Queue full!\n", __func__ );
      return -1;
   }

   printf( "Enqueued 1\n" );

   return 0;
}


void *_msgQueue_RunThread( void *msg )
{
   msg_queue_t qmsg;
   int next_msg;

   while( 1 )
   {
      if ( dequeue_data( msg_queue, &qmsg ) )                      // queue not empty?
      {
         printf( "Outputting %s, %d, %d\n", qmsg.dept, qmsg.alarm, qmsg.level );
         msgSend_PushAlert( qmsg.dept, qmsg.alarm, qmsg.level );

         pthread_mutex_lock( &msgQueue_mutex );
         msgQueue_delay = 100;       // delay 10 seconds between alarm msgs
         pthread_mutex_unlock( &msgQueue_mutex );

         // inter-message delay

         next_msg = 0;
         while( !next_msg )
         {
            pthread_mutex_lock( &msgQueue_mutex );
            if ( --msgQueue_delay <= 0 )
            {
               next_msg = 1;
            }

            pthread_mutex_unlock( &msgQueue_mutex );
            usleep( 100000 );         // delay 1/10 second
         }
      }
      else       // no message ready yet
      {
         usleep( 100000 );         // delay 1/10 second
      }
   }

   return NULL;
}

