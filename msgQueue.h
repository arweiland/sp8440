/** 
 *  @file   msgQueue.h
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/30/15
 *  @brief  Alarm Message queue, header file
 * 
 * @section Description
 * Queues alarm messages from the system and sends them out at timed intervals
 *
 */
#ifndef _MSGQUEUE_H_
#define _MSGQUEUE_H_

int msgQueue_Add( char *msg, int alarm, int level );
void msgQueue_SetDelay( int delay );
void msgQueue_SetAccept( void );

#endif
