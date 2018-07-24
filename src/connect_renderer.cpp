//CREATE CAMERAS here
    ffmpeg_camera camera1(1,argv[1]);
    
    
    
    //CREATE COMPONENTS HERE
    //mmal_engine encoder(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER) 
    //mmal_engine encoder("vc.ril.video_encode");
    //order matters: set_input_port, set_output_port, set_input_flag, set_output_flag, enable
  
    
    
    //Resizer
    //mmal_engine resizer("vc.ril.resize");
    mmal_engine resizer(MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER);
    resizer.set_video_input_port(640,360,MMAL_ENCODING_I420);
    resizer.set_video_output_port(640,360,MMAL_ENCODING_RGB24);
    resizer.enable();
    
    //RENDER //no output port
    mmal_engine renderer(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER);
    renderer.set_video_input_port(640,360,MMAL_ENCODING_I420);
    renderer.enable();

    Connection resizer_renderer(&resizer,&renderer);
    
    resizer_renderer.enable();
    
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
    Buffer jcoder_output((640*360*3)*.5); //(640x360x3)*.5
    
    
    std::mutex m_jcoder;
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      framecount++;
      fprintf(stderr, "Frame numbers %d\n",framecount);
      
      
      //Pass cframe to components here and take the result from _output
      
      
      resizer_renderer.run(&cframe,&jcoder_output);
      //m_jcoder.lock();  //lock outbuffer then read
      
      //m_jcoder.unlock();
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
     
    


