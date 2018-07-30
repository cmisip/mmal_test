
////#ifndef mmal_engine
//#define mmal_engine

//#include "mmal_engine.h"

//#endif\



#ifndef videocapture
#define videocapture


#include "videocapture.h"

#endif



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
    
/*NOTE: Just found out that mmal does not automatically crop the buffers to the original unpadded resolution,
 * so things will not work well if the video width is not a multiple of 32 or height is not a multiple of 16.
 * A cropping algorithm needs to be implemented when copying buffer data for resolutions outside this restriction to work.      
    
/*The specific application of this program can be included from a separate cpp file*/

/*Example parrallel pipeline application using libavcodec to obtain video frames from an rtsp stream
and then sending to three components in parallel: h264 encoder, splitter (as rbg encoder ), jpeg encoder. */

//#include "parallel_app.cpp"                     //WORKING
 
/*Example connecting two components to create one pipeline" */
 
//#include "connections_create_app.cpp"           //WORKING

/*Example connection to the renderer component which does not have any output ports. Default settings for the 
 * output port of the renderer are in mmal_engine.cpp with the variable type MMAL_DISPLAYREGION_T.  */

//#include "connect_renderer_app.cpp"             //WORKING

/*Example to save avcodec frame data to jpeg and PPM. */

//#include "save_avcodec_frame_to_file_app.cpp"   //WORKING

/*Example to save avcodec_frame in YUV420 format to jpeg after transiting through the mmal jpeg encoder. */

//#include "mmal_jpeg_to_file_app.cpp"            //WORKING

/*Example to save avcodec_frame in YUV420 format to ppm after transiting through an appropriate mmal format converter*/

//#include "mmal_rgb_ppm_file_app.cpp"            //WORKING

/*Example to change the video resolution of a frame and then send to the jpeg encoder. */

//#include "resizer_jpeg_app.cpp"                 //WORKING, 

/*Example to chain two connections   */  
    
//#include "connect_connections_app.cpp"          //WORKING,

#include "h264_writer_app.cpp"


};
