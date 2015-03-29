
#ifndef _STARTUP_H_
#define _STARTUP_H_

#include <pthread.h>

// Thread id of main web server
extern pthread_t server_tid;

void MainSignal( int status, char *statstr );
void *sp8440_Start( void *msg );


#endif
