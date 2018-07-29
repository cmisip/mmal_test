
   
    //CREATE CAMERAS here
    ffmpeg_camera camera1(1,argv[1]);
    
    
    
    //CREATE CONNECTIONS HERE
    //Steps
    //1. Setup the mmal_components
    //2. Enable the mmal_components
    //3. Create the connection
    //4. Create the connection's input pool
    //5. Create the connections's output pool
    //6. Enable the connection
    
    
    //splitter
    mmal_engine splitter("vc.ril.video_splitter");
    splitter.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    splitter.set_video_output_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    splitter.enable();
    
    
    //resizer
    mmal_engine resizer("vc.ril.isp");
    resizer.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    resizer.set_video_output_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    resizer.enable();
    
    fprintf(stderr,"Connection splitter_resizer only works if the video width is multiple of 32 and height is multiple of 16");
    Connection splitter_resizer(&splitter,&resizer); 
    splitter_resizer.enable();
    
    
    //resizer2
    mmal_engine resizer2("vc.ril.isp");
    resizer2.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    resizer2.set_video_output_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    resizer2.enable();
    
    
    //renderer
    //no output port
    mmal_engine renderer(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER);
    renderer.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    renderer.enable();
    
    
    Connection resizer2_renderer(&resizer2,&renderer);
    resizer2_renderer.enable();
    
    
    Connection fullpipe(&splitter_resizer,&resizer2_renderer);
    fullpipe.create_input_pool();
    fullpipe.create_output_pool();
    fullpipe.enable();
    
    
    
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
    Buffer jcoder_output(((camera1.get_width()*camera1.get_height()*12)/8)); 
    
    
    std::mutex m_splitter_jcoder;
    fprintf(stderr, "If this works, you should be seeing a video\n");
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      framecount++;
      fprintf(stderr, "Frame numbers %d\n",framecount);
      
      
      //Pass cframe to components here and take the result from _output
      
      
      fullpipe.run(&cframe,&jcoder_output);
      
      m_splitter_jcoder.lock();  //lock outbuffer then read
      
      m_splitter_jcoder.unlock();
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
     
    






