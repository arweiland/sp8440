/** 
 *  @file msgBuild.h
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  phone HTML msg builder header file
 *
 *  @section Description
 * 
 * 
 * Using HTML template and parameters, creates messages ready to send to phone
 *
 */


#ifndef _MSGBUILD_H_
#define _MSGBUILD_H_

extern int msgBuild_makeAlertMsg( char *template_fname, char *outbuf, int bufsize, char *dept, int alarm_num, int level );
extern int msgBuild_makeAcceptMsg( char *template_fname, char *outbuf, int bufsize, char *dept, char *msg );

#endif
