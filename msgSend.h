/** 
 *  @file   msgSend.h
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Message sender header file
 *
 *  @section DESCRIPTION 
 * 
 * Creates and sends the HTML messages to the phones in parallel using libcurl
 *
 */


#ifndef _MSGSEND_H_
#define _MSGSEND_H_

void msgSend_PushMsgs( char *dept, int level );     // send message to all available phones


#endif
