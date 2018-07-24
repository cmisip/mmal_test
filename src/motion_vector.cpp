
////#ifndef mmal_engine
//#define mmal_engine

//#include "mmal_engine.h"

//#endif

//#ifndef connection
//#define connection

#include "connection.h"

//#endif

#include <fcntl.h>
#include <mutex>


//Add headers for specific applications





int main(int argc, char **argv) {
	if (argc != 2) {
        fprintf(stderr, "Usage: %s rtsp://<user>:<pass>@url\n", argv[0]);
        return 1;
    }
/*The specific application of this program can be included from a separate cpp file

  Example parrallel pipeline application using libavcodec to obtain video frames from an rtsp stream
and then sending to three components in parallel: h264 encoder, splitter (as rbg encoder ), jpeg encoder. */
//#include "parallel_app.cpp"    

//#include "connections_create_app.cpp"

#include "connect_renderer.cpp"
 

};
