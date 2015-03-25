
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

void Log( int level, char *format, ... ) __attribute__(( format (printf, 2, 3 )));

#endif
