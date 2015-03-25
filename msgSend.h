/** 
 *  @file   msgSend.h
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Message sender header file
 * 
 *  @section Description
 *  Creates and sends the HTML messages to the phones in parallel using libcurl
 *
 */


#ifndef _MSGSEND_H_
#define _MSGSEND_H_

#define MSGSEND_ACCEPT    0
#define MSGSEND_COMPLETE  1

void msgSend_PushAlert( char *dept, int alarm, int level );     // send Alert message to all available phones
void msgSend_PushAccept( char *dept, int type );     // send Accept or complete message to all available phones

#endif
