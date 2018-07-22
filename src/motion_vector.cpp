
#include "mmal_engine.h"
#include <fcntl.h>
#include <mutex>




int main(int argc, char **argv) {
	if (argc != 2) {
        fprintf(stderr, "Usage: %s rtsp://<user>:<pass>@url\n", argv[0]);
        return 1;
    }
    
    
    //CREATE CAMERAS here
    ffmpeg_camera camera1(1,argv[1]);
    
    
    
    //CREATE COMPONENTS HERE
    //mmal_engine encoder(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER) 
    //mmal_engine encoder("vc.ril.video_encode");
    
    //H264 encoder
    mmal_engine encoder(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER);
  
    encoder.set_input_flag(MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS);
    encoder.set_input_port(640,360,MMAL_ENCODING_I420);
    encoder.set_output_port(640,360,MMAL_ENCODING_H264);
    
    encoder.enable();
    
    
    //RGB encoder
    mmal_engine rgbcoder(MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER);
  
    rgbcoder.set_input_port(640,360,MMAL_ENCODING_I420);
    rgbcoder.set_output_port(640,360,MMAL_ENCODING_RGB24);
    
    rgbcoder.enable();
    
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
    int max_frames=100;
    char c;
    int n, tem;
    
    
    tem = fcntl(0, F_GETFL, 0);
    fcntl (0, F_SETFL, (tem | O_NDELAY));
    
    //motion vector
    uint32_t encoder_bufsize=((640*368)/256)*4;
    uint8_t *encoder_output=(uint8_t *)malloc(encoder_bufsize); //((640x368)/256)*4
    
 
    //rgb24 buffer
    uint32_t rgbcoder_bufsize=((640*360)*3);
    uint8_t *rgbcoder_output=(uint8_t *)malloc(rgbcoder_bufsize); //((640x368)/256)*4
    
    
    //jpeg at 50% compression
    uint32_t jcoder_bufsize=(640*360*3)*.5;
    uint8_t *jcoder_output=(uint8_t *)malloc(jcoder_bufsize); //(640x360x3)*.5
    
    
    std::mutex m_encoder, m_jcoder, m_rgbcoder;
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      framecount++;
      fprintf(stderr, "Frame number %d\n",framecount);
      
      
      //Pass cframe to components here and take the result from _output
      
      encoder.run(&cframe,&encoder_output, encoder_bufsize);
      //m_encoder.lock(); //lock outbuffer then read
      
      //m_encoder.unlock();
      
      
      rgbcoder.run(&cframe,&rgbcoder_output, rgbcoder_bufsize);
      //m_rgbcoder.lock(); //lock outbuffer then read
      
      //m_rgbcoder.unlock();
      
      
      
      jcoder.run(&cframe,&jcoder_output, jcoder_bufsize);
      //m_jcoder.lock();  //lock outbuffer then read
      
      //m_jcoder.unlock();
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
     
    free(encoder_output);
    free(rgbcoder_output);
    free(jcoder_output);
	
}
