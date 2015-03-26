/** 
 *  @file   logging.h
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Handles module logging duties, header file
 *
 */

#ifndef _LOGGING_H_
#define _LOGGING_H_

typedef enum
{
   EMERG,
   ALERT,
   CRIT,
   ERROR,
   WARN,
   NOTICE,
   INFO,
   DEBUG
}alarm_levels_s;

void PLog( int level, char *format, ... ) __attribute__(( format (printf, 2, 3 )));
void Log( int level, char *format, ... ) __attribute__(( format (printf, 2, 3 )));

#endif
