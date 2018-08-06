
   
    
    
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

    //CONTAINER READER SETUP
    MMAL_STATUS_T status = MMAL_EINVAL;
    MMAL_COMPONENT_T *reader = 0;
    if (mmal_component_create(MMAL_COMPONENT_DEFAULT_CONTAINER_READER,&reader)  != MMAL_SUCCESS ){
       fprintf(stderr,"Could not create reader component\n");
        return status;
    }
    
    MMAL_PARAMETER_SEEK_T seek = {{MMAL_PARAMETER_SEEK, sizeof(MMAL_PARAMETER_SEEK_T)},0,0};
    MMAL_PARAMETER_STRING_T *param = 0;
    unsigned int param_size, track_audio, track_video;
    uint32_t encoding;
    size_t uri_len;

    if(!reader->output_num) {
      fprintf(stderr, "%s doesn't have output ports\n", reader->name);
      return status;
    }

    
    /* Open the given URI */
    const char * uri = argv[1];
    uri_len = strlen(uri);
    param_size = sizeof(MMAL_PARAMETER_STRING_T) + uri_len;
    param = (MMAL_PARAMETER_STRING_T *)malloc(param_size);
    if(!param) {
      fprintf(stderr,"out of memory\n");
      status = MMAL_ENOMEM;
      return status;
    }
    
    memset(param, 0, param_size);
    param->hdr.id = MMAL_PARAMETER_URI;
    param->hdr.size = param_size;
    vcos_safe_strcpy(param->str, uri, uri_len + 1, 0);
    if (mmal_port_parameter_set(reader->control, &param->hdr) != MMAL_SUCCESS ) {
	  fprintf(stderr,"%s could not open file %s\n", reader->name, uri);	
	  return status;
	}	
    status = MMAL_SUCCESS;
    
    MMAL_PORT_T *reader_video=NULL, *reader_audio=NULL;
    /* Look for a video track */
    for(track_video = 0; track_video < reader->output_num; track_video++)
      if(reader->output[track_video]->format->type == MMAL_ES_TYPE_VIDEO) break;
      if(track_video != reader->output_num)
        reader_video = reader->output[track_video];
      
    
    for(track_audio = 0; track_audio < reader->output_num; track_audio++)
      if(reader->output[track_audio]->format->type == MMAL_ES_TYPE_AUDIO) break;
      if(track_audio != reader->output_num)
      reader_audio = reader->output[track_audio];
      
      
    if(track_video == reader->output_num && track_audio == reader->output_num){
        fprintf(stderr,"no track to decode\n");
        status = MMAL_EINVAL;
        return status;
    }
      
    fprintf(stderr,"----Reader input port format-----\n");
    if(track_video != reader->output_num)
       mmal_engine::log_format(reader->output[track_video]->format, 0);
    if(track_audio != reader->output_num)
       mmal_engine::log_format(reader->output[track_audio]->format, 0);  
      
    /* If we want to seek to a position in the input file
    if(ctx->options.seeking)
   {
      LOG_DEBUG("seek to %fs", ctx->options.seeking);
      seek.offset = ctx->options.seeking * INT64_C(1000000);
      status = mmal_port_parameter_set(reader->control, &seek.hdr);
      if(status != MMAL_SUCCESS)
         LOG_ERROR("failed to seek to %fs", ctx->options.seeking);
   }*/  
      
      
    //DECODER SETUP
    //H264 encoder
    mmal_engine _decoder(MMAL_COMPONENT_DEFAULT_VIDEO_DECODER);
    MMAL_COMPONENT_T * decoder=_decoder.engine;
    _decoder.set_video_input_port(reader->output[track_video]->format->es->video.width,reader->output[track_video]->format->es->video.height,MMAL_ENCODING_H264);
    _decoder.set_video_output_port(reader->output[track_video]->format->es->video.width,reader->output[track_video]->format->es->video.height,MMAL_ENCODING_I420);  
    //_decoder.enable_video_input_port();
    //_decoder.enable_video_output_port();
 
    //_decoder.create_output_pool();
    //_h264coder.create_input_pool();

    _decoder.enable();  
    
    //CREATE CONNECTION
    MMAL_CONNECTION_T * connection;
    if (mmal_connection_create(&connection, reader->output[track_video], decoder->input[0], MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_OUTPUT) != MMAL_SUCCESS ) {
		fprintf(stderr,"Failed to create connection-----\n");
		return status;
    }
    
    if (mmal_connection_enable(connection) != MMAL_SUCCESS ) {
	   fprintf(stderr,"Failed to enable connection \n");	
	}	
	
    MMAL_POOL_T *output_pool=0, *input_pool=0; 
    
	output_pool = mmal_pool_create(connection->out->buffer_num,
                                  connection->out->buffer_size);
                                  
                                  
    static struct CONTEXT_T {
      VCOS_SEMAPHORE_T semaphore;
      MMAL_QUEUE_T *queue;
     } contextop;
     
     contextop.queue = mmal_queue_create();
    
    decoder->output[0]->userdata = (struct MMAL_PORT_USERDATA_T *)&contextop; 
     
    if (mmal_port_enable(decoder->output[0], Connection::output_callback) != MMAL_SUCCESS) {
	  fprintf(stderr, "decoder output not enabled");	
	}	
    
    
    
    
    
    getchar();
    getchar();
    
    /* Start decoding */
    fprintf(stderr, "start encoding\n");
    
    int framecount=0;
    int max_frames=10;
    char c;
    int n, tem;
    
    
    
    tem = fcntl(0, F_GETFL, 0);
    fcntl (0, F_SETFL, (tem | O_NDELAY));
    
    
    //Buffer creation, make sure buffers are larger than the largest mmal buffer size expected
    
    
    //h264 at 50% compression
    //Buffer h264coder_output((camera1.get_width()*camera1.get_height()*3)*.5); 
    
    
    std::mutex m_h264coder;
   
   
    
    
    while (framecount < max_frames) {
     
      
      framecount++;
      fprintf(stderr, "Frame number %d\n",framecount);
      
      
      MMAL_BUFFER_HEADER_T *buffer;
      MMAL_STATUS_T status;
      
      //if ((buffer = mmal_queue_get(input_pool->queue)) != NULL)  {
                    
                    
                //  mmal_buffer_header_mem_lock(buffer);
                  //av_image_copy_to_buffer(buffer->data, buffsize, (const uint8_t **)(*frame)->data, (*frame)->linesize,
                               // AV_PIX_FMT_YUV420P, (*frame)->width, (*frame)->height, 1);
                  //buffer->length=buffsize;                                                                     //if we supply a time stamp to pts, the first buffer returned with the same time stamp is the matching data for the frame sent
                  //mmal_buffer_header_mem_unlock(buffer);
                       
                //  fprintf(stderr, " sending >>>>> bytes\n");
                  //status = mmal_port_send_buffer(reader->input[0], buffer);
                  //CHECK_STATUS(status, "failed to send buffer")     
                 //}
      
      
      //vcos_semaphore_wait(&context.event);
      //buffer = mmal_queue_get(connection->queue);
     // while (buffer) {
      //while ((buffer = mmal_queue_get(context.queue)) != NULL) {
      while ((buffer = mmal_queue_get(contextop.queue)) != NULL) {
					mmal_buffer_header_mem_lock(buffer);
                    fprintf(stderr, "%s receiving %d bytes <<<<< frame\n", connection->out->name, buffer->length);
         
                    /*if (outbuf) {
                      memset(outbuf->data,0,outbuf->length);
                      memcpy(outbuf->data,buffer->data,buffer->length);
                      outbuf->length=buffer->length;
                      outbuf->flags=buffer->flags;
                      outbuf->cmd=buffer->cmd;
                      outbuf->pts=buffer->pts;
                      outbuf->dts=buffer->dts;
				    }*/
                    mmal_buffer_header_mem_unlock(buffer); 
                    mmal_buffer_header_release(buffer);  
					  
					  
	   }	
			
	   //buffer = connection->pool ? mmal_queue_get(connection->pool->queue) : NULL;		  
	   //while (buffer) {
	   while ((buffer = mmal_queue_get(output_pool->queue)) != NULL) {
                    if (mmal_port_send_buffer(connection->out, buffer) != MMAL_SUCCESS) {
                        fprintf(stderr, "failed to send buffer\n");
				    } else
				        fprintf(stderr, "successful sent buffer to output port\n");
       }			  
   
      
    
     
      
      
      
      n = read(0, &c, 1);
        if (n > 0) break;
    }
    
    fcntl(0, F_SETFL, tem);
    /* End decoding */
    if(param)
      free(param);
   
    fprintf(stderr, "end encoding\n");
    
    
    
     
    





