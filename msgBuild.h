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

/** @brief Creates HTML alert message from template and data ready to send to phone
 *
 * @param template_fname Name of alert template file
 * @param outbuf Location to put completed message
 * @param bufsize Size of outbuf data area
 * @param dept Pointer to department name
 * @param alarm_num Alarm number
 * @param level Alarm Escalation level
 * @return 0 if OK, non-zero if error
 */
int msgBuild_makeAlertMsg( char *template_fname, char *outbuf, int bufsize, char *dept, int alarm_num, int level );

/** @brief Creates HTML acceptance message from template and data ready to send to phone
 *
 * @param template_fname Name of alert template file
 * @param outbuf Location to put completed message
 * @param bufsize Size of outbuf data area
 * @param msg Message to put after department on phone screen
 * @return 0 if OK, non-zero if error
 */
int msgBuild_makeAcceptMsg( char *template_fname, char *outbuf, int bufsize, char *dept, char *msg );

#endif
