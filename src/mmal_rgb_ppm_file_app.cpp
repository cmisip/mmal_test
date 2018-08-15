
    
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
    mmal_engine rgbcoder("vc.ril.isp");

    rgbcoder.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    rgbcoder.set_video_output_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_RGB24);

    rgbcoder.enable_video_input_port();
    rgbcoder.enable_video_output_port();
 
    rgbcoder.create_output_pool();
    rgbcoder.create_input_pool();

    rgbcoder.enable();
    
    
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
    Buffer rgbcoder_output((camera1.get_width()*camera1.get_height()*3)*.9); //(640x360x3)*.5
    
    
    std::mutex m_rgbcoder;
   
    
    FILE *pFile;
    char szFilename[32];
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      framecount++;
      fprintf(stderr, "Frame number %d\n",framecount);
      
      
      //Pass cframe to components here and take the result from _output
      
      rgbcoder.run(&cframe,&rgbcoder_output);
      m_rgbcoder.lock();  //lock outbuffer then read
      
      
      // Open file
      sprintf(szFilename, "frame.ppm", framecount);
      pFile=fopen(szFilename, "wb");
      
      // Write header
      fprintf(pFile, "P6\n%d %d\n255\n", camera1.get_width(), camera1.get_height());

      // Write pixel data
      fwrite(rgbcoder_output.data, 1, rgbcoder_output.length, pFile);    

      // Close file
      fclose(pFile);
      m_rgbcoder.unlock();
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
    
    
    
     
    





