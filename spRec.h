/** 
 *  @file   spRec.h
 *  @author Ron Weiland, Indyme Solutions
 *  @date   3/13/15
 *  @brief  Handles phone status
 *
 *  @section DESCRIPTION 
 * 
 * Contains code to track the status of phones.
 * Saves current status in JSON in case of reboot.
 *
 */


#ifndef _SPREC_H_
#define _SPREC_H_

#define MAX_MAC      20
#define MAX_IP_ADDR  20

typedef struct
{
   int in_use;                         // True if structure is in use, false if free, -1 if past last
   char mac[MAX_MAC];                  // MAC address of phone
   char ip_addr[MAX_IP_ADDR];          // IP address of phone
   int line_number;                    // Line number of phone
   int last_seen;                      // When last seen (UTC)
}SPphone_record_t;


void spRec_Init( void );
SPphone_record_t *spRec_GetNextRecord( SPphone_record_t *sptr );
int spRec_AddRecord( char *ip_addr, char *mac, int line_num );
void spRec_RemoveIP( char *ip_addr );
SPphone_record_t *spRec_FindIP( char *ip_addr );

#endif
