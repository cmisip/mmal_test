
    
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
    
    /*NOTES:
      1. Only works with rtsp streams, file streams are processed but with a lot of pixelation and artifacts.
      * Maybe in bytestream, the NAL's are truncated when more than one comprise a frame.  With rtpstream, I 
      * think the payload is a complete NAL unit.  
      2. Only works with lower resolutions.  This could be due to the configured GPU mem.
      3. There might be other unforeseen interactions between avcodec and mmal.  Right now, mp4 writing is done
      * by avcodec including the header info. 
      4. When video is played with mplayer, every now and then there is "Error while decoding frame!",
      * but there is no glitch in the playback. 
      5. Only tested with rtsp stream at 320x192. 
   
    */
    //H264 encoder
    mmal_engine _h264coder(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER);
    
    //Direct access to the engine
    MMAL_COMPONENT_T * h264coder=_h264coder.engine;
                                            
    h264coder->output[0]->format->bitrate = 25000000;
    
    
    _h264coder.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    _h264coder.set_video_output_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_H264);
    
    MMAL_STATUS_T status;
    
    {
    MMAL_PARAMETER_VIDEO_PROFILE_T param = {{MMAL_PARAMETER_PROFILE, sizeof(param)}, MMAL_VIDEO_PROFILE_H264_HIGH, MMAL_VIDEO_LEVEL_H264_4};
    
    
    if (mmal_port_parameter_set(h264coder->output[0], &param.hdr) != MMAL_SUCCESS ){
      fprintf(stderr,"H264: Could not SET PROFILE TO HIGH\n");
      return status;
    }
    }
    
    if (mmal_port_parameter_set_boolean(h264coder->output[0], MMAL_PARAMETER_VIDEO_ENCODE_INLINE_HEADER, 1) != MMAL_SUCCESS) {
      fprintf(stderr,"H264: Could not set INLINE HEADER\n");
      return status;
    }
    
    {
    MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_INTRAPERIOD, sizeof(param)}, 30};
    if (mmal_port_parameter_set(h264coder->output[0], &param.hdr) != MMAL_SUCCESS) {
      fprintf(stderr,"H264: Could not set INTRAPERIOD\n");
      return status; 
    }
    }
    {
    MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_VIDEO_ENCODE_INITIAL_QUANT, sizeof(param)}, 22};
    if (mmal_port_parameter_set(h264coder->output[0], &param.hdr) != MMAL_SUCCESS ) {
	  fprintf(stderr,"H264: Could not set INITIAL_QUANT\n");
      return status;  
	}	
    }
    
    {
    MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_VIDEO_ENCODE_MAX_QUANT, sizeof(param)}, 22};
    if (mmal_port_parameter_set(h264coder->output[0], &param.hdr) != MMAL_SUCCESS ) {
	  fprintf(stderr,"H264: Could not set MAX_QUANT\n");
      return status;  
	}	
    }
    
    {
    MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_VIDEO_ENCODE_MIN_QUANT, sizeof(param)}, 22};
    if (mmal_port_parameter_set(h264coder->output[0], &param.hdr) != MMAL_SUCCESS ) {
	  fprintf(stderr,"H264: Could not set MIN_QUANT\n");
      return status;  
	}	
    }
    
    _h264coder.enable_video_input_port();
    _h264coder.enable_video_output_port();
 
    _h264coder.create_output_pool();
    _h264coder.create_input_pool();

    _h264coder.enable();
    
    
    
    getchar();
    getchar();
    
    /* Start decoding */
    fprintf(stderr, "start encoding\n");
    
    int framecount=0;
    int max_frames=1000;
    char c;
    int n, tem;
    
    
    
    tem = fcntl(0, F_GETFL, 0);
    fcntl (0, F_SETFL, (tem | O_NDELAY));
    
    
    //Buffer creation, make sure buffers are larger than the largest mmal buffer size expected
    
    
    //h264 at 50% compression
    Buffer h264coder_output((camera1.get_width()*camera1.get_height()*3)*.5); 
    
    
    std::mutex m_h264coder;
   
    camera1.Init_MP4("camcorder.mp4");  
    
   
    while (framecount < max_frames) {
      AVFrame *cframe = camera1.run();
      
      framecount++;
      fprintf(stderr, "Frame number %d\n",framecount);
      
      
      //Pass cframe to components here and take the result from _output
      
      _h264coder.run(&cframe,&h264coder_output);
      
      m_h264coder.lock();  //lock outbuffer then read
      
      if (h264coder_output.flags & MMAL_BUFFER_HEADER_FLAG_CONFIG) {
       fprintf(stderr, "GOT HEADER %d \n", framecount);
       //Not using, MP4 writer uses avcodec to write the header
      
      } 
	  
	  if (h264coder_output.flags & MMAL_BUFFER_HEADER_FLAG_FRAME_START) {
	    fprintf(stderr, "START------------------------------ %d \n", framecount);
	  //Not seeing this  	  
	  }	  
	  
	  if (h264coder_output.flags & MMAL_BUFFER_HEADER_FLAG_KEYFRAME) {
	    fprintf(stderr, "KEYFRAME------------------------------ %d \n", framecount);
	     //KEYFRAME also sends FRAME END so it is written by the next line
	     
	  }	  
	 
	  if (h264coder_output.flags & MMAL_BUFFER_HEADER_FLAG_FRAME_END) {
		  fprintf(stderr, "FRAME END %d  pts = %d  dts = %d\n", framecount, h264coder_output.pts, h264coder_output.dts);
		  camera1.Save_MP4(&h264coder_output,framecount);
	  }
	  
end:
       
      m_h264coder.unlock();
      
      
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    fcntl(0, F_SETFL, tem);
    /* End decoding */
    camera1.Close_MP4();
   
    fprintf(stderr, "end encoding\n");
    
    
    
     
    





