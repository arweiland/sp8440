/** 
 *  @file   msgXML.h
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/23/15
 *  @brief  Phone line registration parser, header file
 * 
 */

#ifndef _MSGXML_H_
#define _MSGXML_H_

typedef struct
{
   char phoneIP[ 20 ];
   char MACAddress[ 30 ];
   char LineNumber[ 10 ];
   char TimeStamp[ 30 ];
}phone_reg_t;

/** @brief Parses telephony registration info from phone
 *
 * @param msg The message received from phone in XML format
 * @param len Total length of message
 * @return Pointer to phone registration data structure
 */
phone_reg_t *msgXML_parseRegistration( char *msg, int len );

#endif
