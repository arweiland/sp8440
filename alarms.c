#include <stdio.h>

// Dummy CLX alarm functions

void escalate_alarm( int alarm_num )
{

}


void ack_alarm_num_no_verify( int alarm_num, int clr_type )
{
   printf( "%s: Acking alarm %d\n", __func__, alarm_num );
}
