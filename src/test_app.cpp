//CREATE CAMERAS here
    ffmpeg_camera camera1(1,argv[1]);
    
    
    
    //CREATE COMPONENTS HERE
    //mmal_engine encoder(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER) 
    //mmal_engine encoder("vc.ril.video_encode");
    //order matters, I think: 
    //1. set_video_input_port
    //2. set_video_output_port
    //3. <optional> set_input_flag, set_output_flag
    //4. enable_video_input_port
    //5. enable_video_output_port
    //6. create_input_pool
    //7. create_output_pool
    //8. enable
    
    
    //RGB encoder
    mmal_engine splitter(MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER);
  
    splitter.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    splitter.set_video_output_port(704,480,MMAL_ENCODING_RGB24);
    
    splitter.enable_video_input_port();
    splitter.enable_video_output_port();
    
    splitter.create_output_pool();
    splitter.create_input_pool();
    
    splitter.enable();
    
   
    
    
    
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
    Buffer jcoder_output((camera1.get_width()*camera1.get_height()*3)*.9); //(640x360x3)*.5
    
    
    std::mutex m_jcoder;
    FILE *JPEGFile;
    char JPEGFName[256];
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      framecount++;
      fprintf(stderr, "Frame number %d\n",framecount);
      
      
      //Pass cframe to components here and take the result from _output
      
     
      
      splitter.run(&cframe,&jcoder_output);
      m_jcoder.lock();  //lock outbuffer then read
      //sprintf(JPEGFName, "frame.jpg", framecount);  //just need one 
      //JPEGFile = fopen(JPEGFName, "wb");
      //fwrite(jcoder_output.data, 1, jcoder_output.length, JPEGFile);
      //fclose(JPEGFile);
      fprintf(stderr, "Cropped to %d bytes", jcoder_output.length);
      m_jcoder.unlock();
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
    
    
    
     
    





