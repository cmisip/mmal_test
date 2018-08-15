
    
    //CREATE CAMERAS here
    ffmpeg_camera camera1(1,argv[1]);
    
    
    //CREATE COMPONENTS HERE
    //mmal_engine encoder(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER) 
    //mmal_engine encoder("vc.ril.video_encode");
    //order matters, I think: 
    //1. set_video_input_port
    //2. set_video_output_port
    
    //3. <optional> set_input_flag, set_output_flag

    //4. create_input_pool
    //5. create_output_pool

    //6. enable_video_input_port
    //7. enable_video_output_port
    //8. enable
       
   
    
    //JPEG encoder
    mmal_engine jcoder(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER);

    jcoder.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    jcoder.set_video_output_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_JPEG);

    jcoder.enable_video_input_port();
    jcoder.enable_video_output_port();
 
    jcoder.create_output_pool();
    jcoder.create_input_pool();

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
    
    
    //jpeg at 10% compression
    Buffer jcoder_output((camera1.get_width()*camera1.get_height()*3)*.9); 
    
    
    std::mutex m_jcoder;
    FILE *JPEGFile;
    char JPEGFName[256];
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      framecount++;
      fprintf(stderr, "Frame number %d\n",framecount);
      
      
      //Pass cframe to components here and take the result from _output
      
      jcoder.run(&cframe,&jcoder_output);
      m_jcoder.lock();  //lock outbuffer then read
      sprintf(JPEGFName, "frame.jpg", framecount);  //just need one 
      JPEGFile = fopen(JPEGFName, "wb");
      fwrite(jcoder_output.data, 1, jcoder_output.length, JPEGFile);
      fclose(JPEGFile);
      m_jcoder.unlock();
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
    
    
    
     
    





