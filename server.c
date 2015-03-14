
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
// Without the following, get "incomplete type" when trying to dereference evhttp_request
#include <event2/http_struct.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


/*---- internal function prototypes ---*/

char *_server_getCmdTypeStr( int type );
void _server_sendAudio( char *root, char *uri, struct evhttp_request *req );

// Note:  evhttp_request is defined in event2/http_struct.h

char *root = ".";         // root directory to service files from


void _server_post_handler( struct evhttp_request *req, void *state )
{
   int cmd;
   size_t len;
   char xmlData[ 500 ];
   struct evbuffer *inBuf;

   cmd = evhttp_request_get_command(req);
   printf( "In post handler.  Command: %s from %s\n", _server_getCmdTypeStr( cmd ), req->remote_host );

   if ( cmd != EVHTTP_REQ_POST )
   {
      printf( "Unexpected request. Type: %s\n", _server_getCmdTypeStr( cmd ) );
      return;
   }

   inBuf = evhttp_request_get_input_buffer(req);
   if ( (len = evbuffer_get_length( inBuf )) > sizeof( xmlData ) )
   {
      printf( "Data too large!. Len: %ld, Max: %ld\n", len, sizeof( xmlData ) );
      return;
   }
   evbuffer_copyout( inBuf, xmlData, len );
   printf( "Post data length: %ld. Data:\n%s", len, xmlData );
   evhttp_send_reply(req, HTTP_OK, "OK", NULL);
}


void activate_handler(struct evhttp_request *req, void *state)
{
   struct evkeyvalq headers;
   char *val;

   printf( "Activate Ack handler called! URI: req->uri\n" );

   printf( "activate. Uri: %s\n", req->uri );

   evhttp_parse_query( req->uri, &headers );

   if ( (val = evhttp_find_header( &headers, "response" )) != NULL )
   {
      printf( "Action: %s\n", val );
   }
   else
   {
      printf( "Action not found\n" );
   }

   if ( (val = evhttp_find_header( &headers, "alarm" )) != NULL )
   {
      printf( "Alarm: %s\n", val );
   }
   else
   {
      printf( "Alarm not found\n" );
   }

   evhttp_send_reply(req, HTTP_OK, "OK", NULL);
}


void _server_generic_handler(struct evhttp_request *req, void *state)
{
   struct evbuffer *buf;
   char *str;
   const char *uri;
   int cmd;
//    struct evhttp_uri *parsed;
//    const char *path;
//    char *decoded_path;

   uri = evhttp_request_get_uri(req);         // Get full URI of request
   cmd = evhttp_request_get_command(req);     // Get command type (GET, POST, etc)
   buf = evbuffer_new();                      // create response buffer

   printf( "Command type: %s\n", _server_getCmdTypeStr( cmd ) );
   printf( "Received request from %s: URI: %s\n", req->remote_host, uri );

#if 0
   str = evhttp_decode_uri( evhttp_request_get_uri(req) );
   printf( "Decoded URI:  %s\n", str );
#else
   str = req->uri;
#endif


   if ( strcasestr( str, ".wav" ) )
   {
      _server_sendAudio( root, str, req );
   }
   else
   {
      evbuffer_add_printf(buf, "Server Responsed. Requested: %s\n", evhttp_request_get_uri(req));
      evhttp_send_reply(req, HTTP_OK, "OK", buf);
   }

#if 0
   free( str );
#endif

/*
    parsed = evhttp_uri_parse( uri );
    if (!parsed)
    {
       printf( "Parsed: Not a good URI\n" );
    }

    path = evhttp_uri_get_path( parsed );
    if (!path)
    {
       printf( "No path.  Using \"\\\"\n" );
       path = "/";
    }

    decoded_path = evhttp_uridecode(path, 0, NULL);
    if (decoded_path == NULL)
    {
       printf( "Decoded path is NULL\n" );
    }
    else
    {
       printf( "Decoded path: %s\n", decoded_path );
    }

    if(!buf)
    {
        puts("failed to create response buffer \n");
        return;
    }

    if( parsed)
    {
       evhttp_uri_free( parsed );
    }
    if ( decoded_path )
    {
       free(decoded_path);
    }
*/

    evbuffer_free(buf);
    printf( "\n");
}

int main(int argc, char* argv[])
{
   char *errStr;

   short          http_port = 8080;
   char          *http_addr = INADDR_ANY;

   struct event_base * base = event_base_new();

   struct evhttp * http_server = evhttp_new(base);
   if(!http_server)
   {
      return -1;
   }

   int ret = evhttp_bind_socket(http_server,http_addr,http_port);
   if(ret!=0)
   {
      errStr = strerror( errno );
      printf( "Bind failed!: %s\n", errStr );
      return -1;
   }

   /* Set a callback for requests to "/specific". */
   /* evhttp_set_cb(httpd, "/specific", another_handler, NULL); */
   /* Set a callback for all other requests. */

   evhttp_set_cb( http_server, "/i_activate", activate_handler, NULL );
   evhttp_set_cb( http_server, "/", _server_post_handler, NULL );
   evhttp_set_gencb(http_server, _server_generic_handler, NULL);

   printf("http server start OK! \n");

   event_base_dispatch(base);

   evhttp_free(http_server);

//   WSACleanup();
   return 0;
}



char *_server_getCmdTypeStr( int type )
{
   char *cmdtype;

	switch (type) 
   {
      case EVHTTP_REQ_GET: cmdtype = "GET"; break;
      case EVHTTP_REQ_POST: cmdtype = "POST"; break;
      case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
      case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
      case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
      case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
      case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
      case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
      case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
      default: cmdtype = "unknown"; break;
	}
   return cmdtype;
}

/*----------------------( _server_sendAudio )---------------------

  Send an audio file back to the requester

  Root is the root directory for files
  uri contains the file name / path
  req is the request

  ------------------------------------------------------------*/

void _server_sendAudio( char *root, char *uri, struct evhttp_request *req )
{
   char fname[100];
   char strbuf[20];
   struct stat st;
   int fd;
   struct evbuffer *evb = NULL;
   struct evkeyvalq *headers;

   sprintf( fname, "%s%s", root, uri );
   if ( (fd = open( fname, O_RDONLY )) == -1 )    // open the file 
   {
      printf( "Can't open file %s: %s\n", fname, strerror( errno ));
      evhttp_send_error(req, 404, "File not found");
      return;
   }

   fstat(fd, &st);
   printf( "File size: %ld\n", st.st_size );

   // Get the headers for the response
   headers = evhttp_request_get_output_headers(req);
   evhttp_add_header(headers, "Content-Type", "audio/x-wav");
   evhttp_add_header(headers, "Accept-Ranges", "bytes");
   sprintf(strbuf, "%ld", st.st_size );
   evhttp_add_header(headers, "Content-Length", strbuf );

   // add the actual audio file to the return data
   evb = evbuffer_new();        // Get the buffer for the return content
   evbuffer_add_file(evb, fd, 0, st.st_size);

   // send it
   evhttp_send_reply( req, 200, "OK", evb );
   evbuffer_free(evb);
}

