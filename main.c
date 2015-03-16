#include <stdio.h>
#include <pthread.h>

#include "spRec.h"
#include "server.h"
#include "msgSend.h"

int main( void )
{
   pthread_t tid;

   spRec_Init();
   server_Init( &tid );

   msgSend_PushMsgs( "Electrical", 2);

   // join on web server thread
   pthread_join( tid, NULL );
   return 0;
}

