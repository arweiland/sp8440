/** 
 *  @file msgBuild.h
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  phone alert msg builder
 *
 *  @section DESCRIPTION 
 * 
 * 
 * Using HTML template, creates message ready to send to phone
 *
 */

#ifndef _MSGBUILD_H_
#define _MSGBUILD_H_

extern int msgBuild_makePushMsg( char *template_fname, char *outbuf, int bufsize, int alarm_num, char *alarm_msg, int level );

#endif
