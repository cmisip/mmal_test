
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
//#include "parallel_ap.cpp"    

//#include "connections_create_ap.cpp"



    
    //CREATE CAMERAS here
    ffmpeg_camera camera1(1,argv[1]);
    
    fprintf(stderr, "Hello");
    
    //CREATE COMPONENTS HERE
    //mmal_engine encoder(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER) 
    //mmal_engine encoder("vc.ril.video_encode");
    //order matters: set_input_port, set_output_port, set_input_flag, set_output_flag, enable
    //H264 encoder
    
    
    //splitter
    mmal_engine splitter(MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER);
    splitter.set_input_port(640,360,MMAL_ENCODING_I420);
    splitter.set_output_port(640,360,MMAL_ENCODING_I420);
    
    //JPEG encoder
    mmal_engine jcoder(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER);
    jcoder.set_input_port(640,360,MMAL_ENCODING_I420);
    jcoder.set_output_port(640,360,MMAL_ENCODING_JPEG);
    
    //Connection splitter_jcoder(&splitter,&jcoder);
    
    //MMAL_CONNECTION_T *connection = 0;
    //mmal_connection_create(&connection, splitter.engine->output[0], jcoder.engine->input[0], MMAL_CONNECTION_FLAG_TUNNELLING);
 
    fprintf(stderr, "There");
    
    //splitter_jcoder.enable();
    
    getchar();
    getchar();
    
    /* Start decoding */
    fprintf(stderr, "start encoding\n");
    
    int framecount=0;
    int max_frames=500;
    char c;
    int n, tem;
    
    
    tem = fcntl(0, F_GETFL, 0);
    fcntl (0, F_SETFL, (tem | O_NDELAY));
    
    
    //Buffer creation, make sure buffers are larger than the largest mmal buffer size expected
   
  
    
    
    //jpeg at 50% compression
    //Buffer jcoder_output((640*360*3)*.5); //(640x360x3)*.5
    
    
    std::mutex m_jcoder;
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      framecount++;
      fprintf(stderr, "Frame number %d\n",framecount);
      
      
      //Pass cframe to components here and take the result from _output
      
      
      //splitter_jcoder.run(&cframe,&jcoder_output);
      m_jcoder.lock();  //lock outbuffer then read
      
      m_jcoder.unlock();
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
     
    







    
    
 
	
}
