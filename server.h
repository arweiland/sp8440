/** 
 *  @file   server.h
 *  @brief  Custom web server based on libevent for Spectralink, header file
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 * 
 * Handles:
 *     - Audio file requests from phone,
 *     - "get" message from phone when soft-key is pressed,
 *     - Telephony notification events.
 */


#ifndef _SERVER_H_
#define _SERVER_H_

#include <pthread.h>

void server_Init( pthread_t *tid );
char *server_GetOurAddress( void );

#endif

