
   
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
    mmal_engine splitter(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER);
    splitter.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    splitter.set_video_output_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    splitter.enable();
    
    //JPEG encoder
    mmal_engine encoder(MMAL_COMPONENT_DEFAULT_CONTAINER_WRITER);
    //jcoder.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    //jcoder.set_video_output_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_H264);
    //jcoder.enable();
    
    encoder.set_video_input_port(camera1.get_width(),camera1.get_height(),VC_CONTAINER_CODEC_H264);
    //encoder.set_video_output_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_H264);
    
    //encoder.set_output_flag(MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS);
    
    encoder.enable_video_input_port();
    //encoder.enable_video_output_port();
    
    //encoder.create_output_pool();
    encoder.create_input_pool();
    
    MMAL_PARAMETER_URI_T *param = 0;
   unsigned int param_size;
   MMAL_STATUS_T status = MMAL_EINVAL;
   size_t uri_len;
   
   
   /* Open the given URI */
   const char *uri = "out.mp4";
   fprintf(stderr,"JEY");
   uri_len = strlen(uri);
   param_size = sizeof(MMAL_PARAMETER_HEADER_T) + uri_len;
   param = (MMAL_PARAMETER_URI_T *)malloc(param_size);
   if(!param)
   {
      //LOG_ERROR("out of memory");
      status = MMAL_ENOMEM;
      //goto error;
   }
   memset(param, 0, param_size);
   param->hdr.id = MMAL_PARAMETER_URI;
   param->hdr.size = param_size;
   vcos_safe_strcpy(param->uri, uri, uri_len + 1, 0);
   status = mmal_port_parameter_set(encoder.engine->control, &param->hdr);   
    
       if(param)
      free(param);
    //Connection splitter_jcoder(&splitter,&jcoder);
    //splitter_jcoder.create_input_pool();
    //splitter_jcoder.create_output_pool();
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
    Buffer jcoder_output((640*360*3)*.5); //(640x360x3)*.5
    
    
    std::mutex m_splitter_jcoder;
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      framecount++;
      fprintf(stderr, "Frame numbers %d\n",framecount);
      
      
      //Pass cframe to components here and take the result from _output
      
      
      //splitter_jcoder.run(&cframe,&jcoder_output);
      m_splitter_jcoder.lock();  //lock outbuffer then read
      
      m_splitter_jcoder.unlock();
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
     
    






