

#ifndef  _QUEUE_H_
   #define  _QUEUE_H_

//#include "environs.h"

#define BOOL int
#define UINT unsigned int

#define  FREE     FALSE
#define  IN_USE   TRUE

#define TRUE  (1==1)
#define FALSE (1==0)

#define MAX_QUEUES  10


/*----------------( queue control structure )--------------------*/
// (one per queue)

typedef struct
{
   BOOL  in_use;              // TRUE if this queue control is in use
   void  *queue_data;         // pointer to queue data array
   void  *queue_data_end;     // end of queue data area
   void  *next_in;            // next input location
   void  *next_out;           // next output location
   int   n_queued;            // number of nodes currently queued
   int   max_queued;          // max depth queue has reached
   int   max_queue_depth;     // max number of nodes in queue
   int   data_size;           // size of one datum
}QUEUE_CTRL;



typedef  QUEUE_CTRL * QUEUE_ID;

QUEUE_ID create_queue( int data_size, int n_data );
QUEUE_ID create_new_queue( void *data, int data_size, int n_data );
void clear_queue( QUEUE_ID queue );
void destroy_queue( QUEUE_ID queue);
BOOL enqueue_data( QUEUE_ID queue, void *data, int data_size );
BOOL dequeue_data( QUEUE_ID queue, void *data );
BOOL read_queue_data( QUEUE_ID queue, void *data );
void *point_queue_data( QUEUE_ID queue );
void *walk_queue( QUEUE_ID queue, void *data );
int  dump_queue_data( QUEUE_ID queue, void *data );
int  get_queue_depth( QUEUE_ID queue );
int  get_queue_max_depth( QUEUE_ID queue );

#endif
