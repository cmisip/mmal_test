
#include "mmal_engine.h"
#include <fcntl.h>




int main(int argc, char **argv) {
	if (argc != 2) {
        fprintf(stderr, "Usage: %s rtsp://<user>:<pass>@url\n", argv[0]);
        return 1;
    }
    
    ffmpeg_camera camera1(1,argv[1]);
    
    //CHOICES
    
    //MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER 
    //mmal_engine encoder("vc.ril.video_encode");
    
    //H264 encoder
    mmal_engine encoder(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER);
  
    encoder.set_input_port(640,360,MMAL_ENCODING_I420);
    encoder.set_output_port(640,360,MMAL_ENCODING_H264);
    
    encoder.enable();
    
    //JPEG encoder
    mmal_engine jcoder(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER);
    jcoder.set_input_port(640,360,MMAL_ENCODING_I420);
    jcoder.set_output_port(640,360,MMAL_ENCODING_JPEG);
    
    jcoder.enable();
    
    
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
    
    
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      framecount++;
      fprintf(stderr, "Frame number %d\n",framecount);
      //Pass cframe to components here
      encoder.run(&cframe);
      jcoder.run(&cframe);
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
     

	
}
