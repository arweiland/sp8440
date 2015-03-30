/** 
 *  @file   startup.h
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/27/15
 *  @brief  Plugin main module for testing
 *
 *  @section Description 
 * 
 * Initializes all modules.\n
 * Starts web server and waits until it dies
 */

#ifndef _STARTUP_H_
#define _STARTUP_H_

#include <pthread.h>

// Thread id of main web server
extern pthread_t server_tid;

void MainSignal( int status, char *statstr );
void *sp8440_Start( void *msg );


#endif
