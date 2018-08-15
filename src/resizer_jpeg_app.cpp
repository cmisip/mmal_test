
    
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

    
    
    
    
    
    //RGB encoder
    mmal_engine resizer("vc.ril.isp");
  
    resizer.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    resizer.set_video_output_port(640,360,MMAL_ENCODING_RGB24);
    
    
    resizer.enable();
    
    //JPEG encoder
    mmal_engine jcoder(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER);

    jcoder.set_video_input_port(640,360,MMAL_ENCODING_I420);
    jcoder.set_video_output_port(640,360,MMAL_ENCODING_JPEG);

    jcoder.enable();
    
    Connection resizer_jcoder(&resizer,&jcoder);
    resizer_jcoder.create_input_pool();
    resizer_jcoder.create_output_pool();
    resizer_jcoder.enable();
    
    
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
    Buffer jcoder_output((704*480*3)*.5); //(640x360x3)*.5
    
    
    std::mutex m_encoder, m_jcoder;
    FILE *JPEGFile;
    char JPEGFName[256];
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      framecount++;
      fprintf(stderr, "Frame number %d\n",framecount);
      
      
      //Pass cframe to components here and take the result from _output
      
      resizer_jcoder.run(&cframe,&jcoder_output);
      m_encoder.lock(); //lock outbuffer then read
      sprintf(JPEGFName, "frame.jpg", framecount);  //just need one 
      JPEGFile = fopen(JPEGFName, "wb");
      fwrite(jcoder_output.data, 1, jcoder_output.length, JPEGFile);
      fclose(JPEGFile);
      m_encoder.unlock();
      
      
      
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
    
    
    
     
    





