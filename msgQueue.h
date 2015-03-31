
#ifndef _MSGQUEUE_H_
#define _MSGQUEUE_H_

int msgQueue_Add( char *msg, int alarm, int level );
void msgQueue_SetDelay( int delay );
void msgQueue_SetAccept( void );

#endif
