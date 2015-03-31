
#ifndef _ALARMS_H_
#define _ALARMS_H_

#define ALARM_PHONE_ACK 2     // dummy value for testing

void escalate_alarm( int alarm_num );
void ack_alarm_num_no_verify( int alarm_num, int clr_type );

#endif
