#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>

//#include "cuexcepts.h"
//#include "coreset.h"
#include "queues.h"


/*-------------------- macro definitions -----------------------*/

#define InitializeCriticalSection(x)
#define EnterCriticalSection( x )       pthread_mutex_lock( x )
#define LeaveCriticalSection( x )       pthread_mutex_unlock( x )

#define get_cur_tid()    0


#define  EXP_QUEUE      0x0200


#define __iassertcheck( errcode, par )  (printf( "error code: %#x, par: %x\n", errcode, par ) )
#define iassert(p, errcode, par) ((p) ? (void)0 : (void) __iassertcheck( errcode, par ) )

/*------------------------- exceptions ------------------------*/

enum
{
   EXP_1 =  EXP_QUEUE,
   EXP_2,
   EXP_3,
   EXP_4,
   EXP_5,
   EXP_6,
   EXP_7,
   EXP_8,
   EXP_9,
   EXP_10,
   EXP_11
};


QUEUE_CTRL queue_ctrls[ MAX_QUEUES ];        // queue control structures
int queues_in_use = 0;                       // number of queues in use


QUEUE_ID _get_new_queue_ctrl( void );

void queues_init( void );


static BOOL queues_initialized = FALSE;

pthread_mutex_t queue_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

/*----------------------( queues_init )---------------------------

  Initialize the queues module

-----------------------------------------------------------------*/

void queues_init( void )
{

   InitializeCriticalSection( &queue_mutex );
   queues_initialized = TRUE;

}



/*----------------------( create_queue )---------------------------

   Create one new queue structure, malloc the data area.

   Inputs:
      data_size:  size of one datum
      n_data:     size of data array

-----------------------------------------------------------------*/

QUEUE_ID create_queue( int data_size, int n_data )
{
  void *data;
  QUEUE_ID ret = 0;

  if ( (data = (void *)malloc( n_data * data_size )) != NULL )
  {
    ret = create_new_queue( data, data_size, n_data );
  }

  return ret;  
  
}




/*--------------------( create_new_queue )-------------------------

   Create one new queue structure.

   Inputs:
      data:       pointer to array of data for queue
      data_size:  size of one datum
      n_data:     size of data array

-----------------------------------------------------------------*/

QUEUE_ID create_new_queue( void *data, int data_size, int n_data )
{
   QUEUE_CTRL *cptr;

   if ( !queues_initialized )
   {
      queues_init();
   }

   EnterCriticalSection( &queue_mutex );

   cptr = _get_new_queue_ctrl();          // get next queue control
   cptr->next_in           = data;
   cptr->next_out          = data;
   cptr->queue_data_end    = (void *)( (char *)data + (data_size * n_data) );
   cptr->n_queued          = 0;
   cptr->max_queued        = 0;                    // no max depth yet

   cptr->max_queue_depth   = n_data;
   cptr->data_size         = data_size;
   cptr->queue_data        = data;

   LeaveCriticalSection( &queue_mutex );

   return( cptr );
}


/*----------------------( clear_queue )---------------------------

  Clear all data from queue, but don't destroy queue

  Inputs:  QUEUE_ID queue

-----------------------------------------------------------------*/

void clear_queue( QUEUE_ID queue )
{

   EnterCriticalSection( &queue_mutex );

   if ( queue != NULL && queue->in_use == IN_USE )
   {
      queue->next_in = queue->queue_data;
      queue->next_out = queue->queue_data;        // Reset pointers back to data
      queue->n_queued = 0;                        // Nothing queued now.
   }

   LeaveCriticalSection( &queue_mutex );
}


/*----------------------( destroy_queue )---------------------------

   Destroy one queue structure, free the data area.

   Inputs:  QUEUE_ID queue

-----------------------------------------------------------------*/

void destroy_queue( QUEUE_ID queue)
{

   EnterCriticalSection( &queue_mutex );

  if ( queue != NULL && queue->in_use == IN_USE )
  {
    if ( queue->queue_data != NULL )              // have data?
    {
      free( queue->queue_data );                  // free it
    }
    memset( queue, 0, sizeof( QUEUE_CTRL ) );     // clear structure
    queue->in_use = FREE;
  }

  LeaveCriticalSection( &queue_mutex );
  
}



/*---------------------( enqueue_data )----------------------------

   Enqueue data into specified queue.  Data is copied into queue
   data.

   Inputs:
      data:       pointer to data to enqueue
      data_size:  size of data to enqueue

   Returns:    TRUE if data enqueued, FALSE if queue is full

-----------------------------------------------------------------*/

BOOL enqueue_data( QUEUE_ID queue, void *data, int data_size )
{
   QUEUE_CTRL  *que_ctrl;
   BOOL  ret;

   que_ctrl = queue;

   /*--------------- make sure data is of legal size ---------------*/

   iassert( que_ctrl->in_use == IN_USE, EXP_1, get_cur_tid() );
   iassert( data_size <= que_ctrl->data_size, EXP_2, data_size );

   /*----------------- make sure queue is not full -----------------*/

   EnterCriticalSection( &queue_mutex );

   if ( que_ctrl->n_queued < que_ctrl->max_queue_depth )
   {
      ret = TRUE;

      /*------------- get address of next insertion point ---------*/

      memcpy( que_ctrl->next_in, data, data_size );      // save data in queue

      que_ctrl->next_in += que_ctrl->data_size;          // next location

      if ( que_ctrl->next_in >= que_ctrl->queue_data_end )
         que_ctrl->next_in = que_ctrl->queue_data;       // wrap queue in ptr

      que_ctrl->n_queued++;

      /*---------- track max depth of buffer pool --------*/

      if( que_ctrl->n_queued > que_ctrl->max_queued )
      {
         que_ctrl->max_queued = que_ctrl->n_queued;
      }

   }

   else
      ret = FALSE;

   LeaveCriticalSection( &queue_mutex );

   return ret;
}



/*---------------------( dequeue_data )----------------------------

   Dequeue next queue data.

   Inputs:
         queue:   id of queue to get data from
         data:    location to put data, NULL if just remove from queue.

   Returns:    TRUE if data enqueued, FALSE if queue is empty

-----------------------------------------------------------------*/

BOOL dequeue_data( QUEUE_ID queue, void *data )
{
  BOOL  ret;
  QUEUE_CTRL  *que_ctrl;
  
  que_ctrl = queue;

  iassert( que_ctrl->in_use == IN_USE, EXP_3, get_cur_tid() );

  EnterCriticalSection( &queue_mutex );

  if ( que_ctrl->n_queued != 0 )
  {
    ret = TRUE;

    if ( data != NULL )
    {
      memcpy( data, que_ctrl->next_out, que_ctrl->data_size ); // pass data
    }

    que_ctrl->next_out += que_ctrl->data_size;              // next location

    if ( que_ctrl->next_out >= que_ctrl->queue_data_end )
    {
      que_ctrl->next_out = que_ctrl->queue_data;            // wrap out ptr
    }

    que_ctrl->n_queued--;

  }
  else
  {
    ret = FALSE;
  }

  LeaveCriticalSection( &queue_mutex );

  return ret;

}



/*---------------------( read_queue_data )-------------------------

   Get next queue data from queue without dequeuing it.

   Inputs:
         queue:   id of queue to get data from
         data:    location to put data

   Returns:    TRUE if data enqueued, FALSE if queue is empty

-----------------------------------------------------------------*/

BOOL read_queue_data( QUEUE_ID queue, void *data )
{
   BOOL  ret;
   QUEUE_CTRL  *que_ctrl;
   
   que_ctrl = queue;

   iassert( que_ctrl->in_use == IN_USE, EXP_4, get_cur_tid() );

   if ( que_ctrl->n_queued != 0 )
   {
      ret = TRUE;

      memcpy( data, que_ctrl->next_out, que_ctrl->data_size ); // pass data
   }
   else
      ret = FALSE;

   return ret;

}



/*---------------------( point_queue_data )-------------------------

   point next queue data from queue without dequeuing it.

   Inputs:
         queue:   id of queue to get data from
         data:    location to put data

   Returns:    pointer to data, NULL if queue is empty.

-----------------------------------------------------------------*/

void *point_queue_data( QUEUE_ID queue )
{
   void *ret = NULL;
   QUEUE_CTRL  *que_ctrl;
   
   que_ctrl = queue;

   iassert( que_ctrl->in_use == IN_USE, EXP_5, get_cur_tid() );

   if ( que_ctrl->n_queued != 0 )
   {
      ret = que_ctrl->next_out;
   }

   return ret;

}



/*---------------------( walk_queue )------------------------------

  Walk through queue.
   
  data is either NULL for first in queue, or the last value
  returned from this call.

  Returns pointer to queue data or NULL if no more in queue.

-----------------------------------------------------------------*/

void *walk_queue( QUEUE_ID queue, void *data )
{
  void *ret;

  // check data pointer somewhere within queue

  iassert( data == NULL ||
    (data >= queue->queue_data && data < queue->queue_data_end), EXP_6, 0 );

  // check data pointer is on queue buffer boundary

  iassert( (((char *)queue->queue_data - (char *)data) % queue->data_size) == 0, EXP_7, 0 );


  EnterCriticalSection( &queue_mutex );

  if ( data == NULL )
  {
    ret = queue->n_queued == 0 ? NULL : queue->next_out;
  }

  else
  {
    ret = (char *)data + queue->data_size;    // point next data

    if ( ret >= queue->queue_data_end )       // past end?
    {
      ret = queue->queue_data;                // wrap it
    }

    if ( ret == queue->next_in )              // at end?
    {
      ret = NULL;                             // no more
    }
  }

  LeaveCriticalSection( &queue_mutex );

  return ret;  
}




/*--------------------( dump_queue_data )--------------------------

   Dump out all the data in the queue without dequeuing it.
   Destination must be large enough to hold the data.

   Inputs:
         queue:   id of queue to get data from
         data:    location to put data


   Returns:    Number of queue elements dumped, FALSE if queue is empty

-----------------------------------------------------------------*/

int dump_queue_data( QUEUE_ID queue, void *data )
{
  QUEUE_CTRL  *que_ctrl;
  void *next_out;
  char *dptr;
  int i;
  int  ret = 0;

  que_ctrl = queue;

  iassert( que_ctrl->in_use == IN_USE, EXP_8, get_cur_tid() );

  EnterCriticalSection( &queue_mutex );

  ret = que_ctrl->n_queued;

  if ( ret != 0 )
  {
    next_out = que_ctrl->next_out;
    dptr = data;

    for ( i = 0; i < ret; i++, dptr += que_ctrl->data_size )
    {
      memcpy( dptr, next_out, que_ctrl->data_size );        // pass data

      next_out += que_ctrl->data_size;                      // next location

      if ( next_out >= que_ctrl->queue_data_end )
      {  
        next_out = que_ctrl->queue_data;                    // wrap out ptr
      }
    }
  }

  LeaveCriticalSection( &queue_mutex );

  return ret;

}



/*---------------------( get_queue_depth )----------------------------

   Get current depth of specified queue
   
   Inputs:
         queue:   id of queue to get data from

   Returns:    current depth of queue

-----------------------------------------------------------------*/

int get_queue_depth( QUEUE_ID queue )
{

   QUEUE_CTRL  *que_ctrl;
   
   que_ctrl = queue;

   iassert( que_ctrl->in_use == IN_USE, EXP_9, get_cur_tid() );

   return( que_ctrl->n_queued );
}




/*------------------( get_queue_max_depth )------------------------

   Get current max depth of specified queue
   
   Inputs:
         queue:   id of queue to get data from

   Returns:    current depth of queue

-----------------------------------------------------------------*/

int get_queue_max_depth( QUEUE_ID queue )
{

   QUEUE_CTRL  *que_ctrl;
   
   que_ctrl = queue;

   iassert( que_ctrl->in_use == IN_USE, EXP_10, get_cur_tid() );

   return( que_ctrl->max_queued );
}



/*------------------( _get_new_queue_ctrl )-----------------------

   Get an available queue control structure.

-----------------------------------------------------------------*/

QUEUE_ID _get_new_queue_ctrl( void )
{

   UINT i;
   QUEUE_CTRL *cptr, *ret;

   cptr = queue_ctrls;                       // point queue controls

   /*--------------- try to find a free control structure -----------*/   

   ret = NULL;

   for ( i = 0; i < MAX_QUEUES; i++, cptr++ )
   {
      if ( cptr->in_use == FREE )
      {
         cptr->in_use = IN_USE;              // this one is in use
         queues_in_use++;                    // one more in use
         ret = cptr;                         // point free control
         break;
      }
   }

   iassert( ret != NULL, EXP_11, queues_in_use );

   return( ( QUEUE_ID )ret );
}

