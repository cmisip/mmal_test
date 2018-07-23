
    
    //CREATE CAMERAS here
    ffmpeg_camera camera1(1,argv[1]);
    
    
    
    //CREATE COMPONENTS HERE
    //mmal_engine encoder(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER) 
    //mmal_engine encoder("vc.ril.video_encode");
    //order matters: set_input_port, set_output_port, set_input_flag, set_output_flag, enable
    //H264 encoder
    mmal_engine encoder(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER);
  
    
    encoder.set_input_port(640,360,MMAL_ENCODING_I420);
    encoder.set_output_port(640,360,MMAL_ENCODING_H264);
    encoder.set_output_flag(MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS);
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
    int max_frames=500;
    char c;
    int n, tem;
    
    
    tem = fcntl(0, F_GETFL, 0);
    fcntl (0, F_SETFL, (tem | O_NDELAY));
    
    
    //Buffer creation, make sure buffers are larger than the largest mmal buffer size expected
    
    //motion vector buffer
    Buffer encoder_output(1884160);
    
 
    //rgb24 buffer
    Buffer rgbcoder_output((640*368)*3); //((640x368)/256)*4
    
    
    //jpeg at 50% compression
    Buffer jcoder_output((640*360*3)*.5); //(640x360x3)*.5
    
    
    std::mutex m_encoder, m_jcoder, m_rgbcoder;
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      framecount++;
      fprintf(stderr, "Frame number %d\n",framecount);
      
      
      //Pass cframe to components here and take the result from _output
      
      encoder.run(&cframe,&encoder_output);
      m_encoder.lock(); //lock outbuffer then read
      if(encoder_output.flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO) {
		 fprintf(stderr, "MOTION VECTOR!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");  
	  }  
      m_encoder.unlock();
      
      
      rgbcoder.run(&cframe,&rgbcoder_output);
      m_rgbcoder.lock(); //lock outbuffer then read
      
      m_rgbcoder.unlock();
      
      
      
      jcoder.run(&cframe,&jcoder_output);
      m_jcoder.lock();  //lock outbuffer then read
      
      m_jcoder.unlock();
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
     
    





