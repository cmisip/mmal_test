
    //CREATE CAMERAS here
    ffmpeg_camera camera1(1,argv[1]);
    extradata_pack *extra=camera1.get_extradata();
    fprintf(stderr,"CHECK EXTRADATA\n");
    extra->print();
    
    
    
    //CREATE CONNECTIONS HERE
    //Steps
    //1. Setup the mmal_components
    //2. Enable the mmal_components
    //3. Create the connection
    //4. Create the connection's input pool
    //5. Create the connections's output pool
    //6. Enable the connection
    
    
    
    
    //H264 decoder
    mmal_engine _decoder(MMAL_COMPONENT_DEFAULT_VIDEO_DECODER);
    _decoder.set_video_input_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_H264,&extra);
    _decoder.set_video_output_port(camera1.get_width(),camera1.get_height(),MMAL_ENCODING_I420);
    
    _decoder.create_output_pool();
    _decoder.create_input_pool();
    
    _decoder.enable_video_input_port();
    _decoder.enable_video_output_port();
    
    
    
    _decoder.enable();
    
    //Direct access to the engine
    MMAL_COMPONENT_T *decoder=_decoder.engine;
                      
    
    /*MMAL_PARAMETER_UINT32_T extra_buffers;
    extra_buffers.hdr.id = MMAL_PARAMETER_EXTRA_BUFFERS;
    extra_buffers.hdr.size = sizeof(MMAL_PARAMETER_UINT32_T);
    extra_buffers.value = 5;
    if (mmal_port_parameter_set(decoder->output[0], &extra_buffers.hdr) < 0)        
       fprintf(stderr, "failed to add extra output buffers");
    */
    
     
   
   
    getchar();
    getchar();
    
    /* Start decoding */
    fprintf(stderr, "start encodings\n");
    
    int framecount=0;
    int max_frames=50;
    char c;
    int n, tem;
    
    
    tem = fcntl(0, F_GETFL, 0);
    fcntl (0, F_SETFL, (tem | O_NDELAY));
    
    
    //Buffer creation, make sure buffers are larger than the largest mmal buffer size expected
    Buffer decoder_output((camera1.get_width()*camera1.get_height()*12)); //I420 is (width*height*12)/8 bytes
    
    
    std::mutex m_decoder;
    
    while (framecount < max_frames) {
	   fprintf(stderr, "Frame numbers %d\n",framecount);
	  
	  	
      AVPacket *cpacket = camera1.get();
      
      //Print the NAL type
      uint8_t nal_type=cpacket->data[4] & 0x1f;
      fprintf(stderr,"NAL TYPE --------------------> %d\n",nal_type);
      
      //Print the Packet Contents
      for (uint32_t i = 0; ((i < cpacket->size) && (i < 50)); ++i) {
                    fprintf(stderr, "\\%02x", (unsigned char)cpacket->data[i]);
      }           
      fprintf(stderr,"\n");
      
	  cpacket->pts=cpacket->dts = MMAL_TIME_UNKNOWN;
	  
      
      
      //Pass cframe to components here and take the result from _output
      _decoder.run(&cpacket,&decoder_output);
      
      av_packet_unref(cpacket);
      
     
      framecount++;
      
      m_decoder.lock();  //lock outbuffer then read
      //operate on the output buff here
      m_decoder.unlock();
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    
    fcntl(0, F_SETFL, tem);
    /* End decding */
    fprintf(stderr, "end encoding\n");
     
    






