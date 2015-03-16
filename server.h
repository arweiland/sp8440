/** 
 *  @file   server.h
 *  @brief  Custom web server based on libevent for Spectralink.
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 * 
 * Handles:
 *     Audio file requests from phone,
 *     "get" message from phone when soft-key is pressed,
 *     Telephony notification events.
 */



#ifndef _SERVER_H_
#define _SERVER_H_

void server_Init( pthread_t *tid );

#endif

