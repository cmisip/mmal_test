#ifndef ffmpeg
#define ffmpeg

#include "ffmpeg_camera.h"

#endif


uint16_t ffmpeg_camera::get_width(){ return video_dec_ctx->width; };
uint16_t ffmpeg_camera::get_height(){ return video_dec_ctx->height; };										  

uint8_t ffmpeg_camera::initialize(){
	
	if (ctype==0) {
		av_dict_set(&opts, "flags2", "+export_mvs", 0);
	}	
	
	avformat_network_init();
    

    av_register_all();

    ret = avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) ;
    if (ret <0 ) {
        fprintf(stderr, "Could not open source %s\n", src_filename);
        return ret;
    }


    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return ret;
    }

	
	ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);  //1 this version creates dec
    
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO), src_filename);
        return ret;
    } else {
        int stream_idx = ret;
        st = fmt_ctx->streams[stream_idx];
        
        dec_ctx = avcodec_alloc_context3(NULL);
        if (!dec_ctx) {
            fprintf(stderr, "Failed to allocate codec\n");
            return AVERROR(EINVAL);
        }

        ret = avcodec_parameters_to_context(dec_ctx, st->codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters to codec context\n");
            return ret;
        }
        dec_ctx->pix_fmt=AV_PIX_FMT_YUV420P; //FIXME, don't know if this does anything
        

         if ( (dec = avcodec_find_decoder_by_name("h264_mmal")) == NULL ) {
               fprintf(stderr, "Can't find codec for video stream");
        } else {
           //Debug(1, "Video Found decoder");
           fprintf(stderr, "Found HW MMAL decoder\n");
        }  

        
        /* Init the video decoder */
        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n", av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
          
            return ret;
        }

        video_stream_idx = stream_idx;
        video_stream = fmt_ctx->streams[video_stream_idx];
        video_dec_ctx = dec_ctx;
        
        fprintf(stderr,"Width: %d\n", video_dec_ctx->width);
        fprintf(stderr,"Height: %d\n", video_dec_ctx->height);
        
	    av_dump_format(fmt_ctx, 0, src_filename, 0);

        if (!video_stream) {
           fprintf(stderr, "Could not find video stream in the input, aborting\n");
           ret = 1;
        }

        
        
        frame = av_frame_alloc();
        if (!frame) {
          fprintf(stderr, "Could not allocate frame\n");
          ret = AVERROR(ENOMEM);
        }

        
        
        //FIXME, START: below code for testing if frames are being captured properly by libavcodec
        //The swscale routine was necessary to save to file using SaveFrame
        pFrameRGB=av_frame_alloc();
        if (!pFrameRGB) {
          fprintf(stderr, "Could not allocate pframe\n");
          ret = AVERROR(ENOMEM);
        }
        
        // Determine required buffer size and allocate buffer
		numBytes=av_image_get_buffer_size(AV_PIX_FMT_RGB24, video_dec_ctx->width, video_dec_ctx->height, 32);
    	      
        rbuffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
        
        // initialize SWS context for software scaling
        sws_ctx = sws_getContext(video_dec_ctx->width,
			   video_dec_ctx->height,
			   video_dec_ctx->pix_fmt,
			   video_dec_ctx->width,
			   video_dec_ctx->height,
			   AV_PIX_FMT_RGB24,
			   SWS_BILINEAR,
			   NULL,
			   NULL,
			   NULL
			   );

	
        } 

       // Assign appropriate parts of buffer to image planes in pFrameRGB
       // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
       // of AVPicture
		 
	   av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize,
            rbuffer, AV_PIX_FMT_RGB24, video_dec_ctx->width, video_dec_ctx->height, 1);
       //FIXME, END   
       
       //FIXME, START : JPEG writing codec
       jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
       if (!jpegCodec) {
         return -1;
       }
       jpegContext = avcodec_alloc_context3(jpegCodec);
       if (!jpegContext) {
         return -1;
       }

       //jpegContext->pix_fmt = video_dec_ctx->pix_fmt;
       jpegContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
       jpegContext->time_base = video_dec_ctx->time_base;
       jpegContext->height = video_dec_ctx->height;
       jpegContext->width = video_dec_ctx->width;

       if (avcodec_open2(jpegContext, jpegCodec, NULL) < 0) {
          return -1;
       }
       //FIXME, END  

return ret;

};

void ffmpeg_camera::Init_MP4(const char* filename){
          
        //Setup outputformatctx for mp4 file output
        //initialize output file
	    //std::string filename = "cameraRecorder.mp4";
	    
        outputFormat = av_guess_format("mp4",filename,NULL);
        if(!outputFormat)
          fprintf(stderr,"ERROR av guess format\n");

        if(avformat_alloc_output_context2(&outputFormatCtx,outputFormat,NULL,filename) < 0 ){
          fprintf(stderr,"Error avformat_alloc_output_context2 \n");
          ret = 1;
        }

        if(!outputFormatCtx){
          fprintf(stderr,"Error alloc output 2 \n");
          ret = 1;
        }
        outCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if( !outCodec ){
          fprintf(stderr,"Error avcodec_find_encoder \n");
          ret = 1;
        }

        outputStream = avformat_new_stream(outputFormatCtx,outCodec);
          if(!outputStream){
          fprintf(stderr,"Error outputStream\n");
          ret = 1;
        }
        
        outputStream->codecpar->codec_id = AV_CODEC_ID_H264;
        outputStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
        
        //outputStream->codec->sample_fmt = outCodec->sample_fmts ?  outCodec->sample_fmts[0] : AV_SAMPLE_FMT_S16;
        
        outputStream->codecpar->width = st->codecpar->width;
        outputStream->codecpar->height = st->codecpar->height;
        outputStream->codecpar->format = AV_PIX_FMT_YUV420P;
        outputStream->codecpar->bit_rate = 4000000;
        outputStream->time_base = AVRational{1,30};
        
        if (outputFormatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
          outputFormatCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
          //fprintf(stderr,"Setting global header flag");
        }
        
        
        if ( !(outputFormatCtx->flags & AVFMT_NOFILE) )
          if( avio_open2(&outputFormatCtx->pb , filename , AVIO_FLAG_WRITE ,NULL, NULL) < 0 ){
            fprintf(stderr,"Error avio_open2");
            ret = 1;
          }

        /*if(avformat_write_header(outputFormatCtx , NULL) < 0){
            fprintf(stderr,"Error avformat_write_header");
            ret = 1;
        }*/
    
        //if (avcodec_open2(outputStream->codec, outCodec, NULL) <0 ) {
        //   fprintf(stderr,"avcodec_open2 error");
        //}
    
    
        fprintf(stderr,"OUTPUT FORMAT-------------------------------");
        av_dump_format(outputFormatCtx , 0 ,filename ,1);
        
        av_init_packet(&pkt);
        	
}	


void ffmpeg_camera::Save_MP4(Buffer *buff, int timeStampValue){
	
	    if  (buff->flags & MMAL_BUFFER_HEADER_FLAG_CONFIG){
			
		   //config headers actually contain SPS and PPS and need to be written in avcc atom of the MP4 header
		   if (!extradata_written) {
		      
		      uint32_t nal_delimiter=0x00000001;
		      uint8_t sps_start=0,sps_end=0,pps_start=0,pps_end=buff->length-1;
		      
		      
		      //find the pps and sps in the bytestring by looking for the nal_delimiter
		      for (uint16_t i=0; i< buff->length; i++ ) {
				 uint32_t tmp=buff->data[i];
				 int result=memcmp(&tmp,&nal_delimiter,4);
				 //fprintf(stderr,"Comparing %.8x to %.8x and result is %d\n", nal_delimiter, (uint32_t)buff->data[i], result);
				 if  (!result ) {
				   //fprintf(stderr,"Match at %d \n", i);
				   if (!sps_start)
				     sps_start=i+1;
				   if (sps_start) {
				     pps_start=i+1;
				     sps_end=i-4;
				     spslen=sps_end-sps_start+1; //start is first byte of sps, end is last byte of sps
				     ppslen=pps_end-pps_start+1; //start is first byte of pps, end is last byte of pps
				   }    
				     
				   
				    
				 }   
			  }	  
			  
			  /*fprintf(stderr,"SPS is %d\n",spslen);
			  for (size_t i = sps_start; i != sps_end+1; ++i)
                    fprintf(stderr, "%02x", (unsigned char)buff->data[i]);
                    fprintf(stderr,"\n");
                    
              fprintf(stderr,"PPS is %d\n",ppslen);
			  for (size_t i = pps_start; i != pps_end+1; ++i)
                    fprintf(stderr, "%02x", (unsigned char)buff->data[i]); 
                    fprintf(stderr,"\n");    
			  */
		      
		      uint8_t nal_unit_type = buff->data[sps_start] & 0x1f;
		      
			  if (nal_unit_type == 7 && !sps)  { //SPS
				  
				  sps=(uint8_t *)av_mallocz(spslen);
				  memcpy(sps,(void*)&buff->data[sps_start],spslen);
				  
				  /*fprintf(stderr,"GOT THE SPS %d \n",buff->length);
				  for (size_t i = 0; i != spslen; ++i)
                    fprintf(stderr, "%02x", (unsigned char)sps[i]);
                    fprintf(stderr,"\n"); 
				  */
			  }	
			    
	          
				  
			  nal_unit_type = buff->data[pps_start] & 0x1f;	

			  if (nal_unit_type == 8 && !pps)  { //PPS  
				  pps=(uint8_t *)av_mallocz(ppslen); 
				  memcpy(pps,(void*)&buff->data[pps_start],ppslen);
				   
				  /*fprintf(stderr,"GOT THE PPS\n");
				  for (size_t i = 0; i != ppslen; ++i)
                    fprintf(stderr, "%02x", (unsigned char)pps[i]);
			        fprintf(stderr,"\n");
			      */  
			  }	  
	       
	       if ((sps) && (pps)) {
			  //length of extradata is 6 bytes + 2 bytes for spslen + sps + 1 byte number of pps + 2 bytes for ppslen + pps
			   
			  uint32_t extradata_len = 8 + spslen + 1 + 2 + ppslen;
			  outputStream->codecpar->extradata = (uint8_t*)av_mallocz(extradata_len);
			  
              outputStream->codecpar->extradata_size = extradata_len;

              //start writing avcc extradata
              outputStream->codecpar->extradata[0] = 0x01;      //version
              outputStream->codecpar->extradata[1] = sps[1];    //profile
              outputStream->codecpar->extradata[2] = sps[2];    //comatibility
              outputStream->codecpar->extradata[3] = sps[3];    //level
              outputStream->codecpar->extradata[4] = 0xFC | 3;  // reserved (6 bits), NALU length size - 1 (2 bits) which is 3
              outputStream->codecpar->extradata[5] = 0xE0 | 1;  // reserved (3 bits), num of SPS (5 bits) which is 1 sps
              
              //write spslen in big endian
              outputStream->codecpar->extradata[6] = (spslen >> 8) & 0x00ff;
              outputStream->codecpar->extradata[7] = spslen & 0x00ff;
              
              //Write the actual sps
              int i = 0;
              for (i=0; i<spslen; i++) {
                outputStream->codecpar->extradata[8 + i] = sps[i];
              }
              
              /*for (size_t i = 0; i != outputStream->codecpar->extradata_size; ++i)
                    fprintf(stderr, "\\%02x", (unsigned char)outputStream->codecpar->extradata[i]);
                    fprintf(stderr,"\n");
              */
              
              //Number of pps
              outputStream->codecpar->extradata[8 + spslen] = 0x01;
              
              
              /*for (size_t i = 0; i != outputStream->codecpar->extradata_size; ++i)
                    fprintf(stderr, "\\%02x", (unsigned char)outputStream->codecpar->extradata[i]);
                    fprintf(stderr,"\n");
              */
              
              //Size of pps in big endian
              outputStream->codecpar->extradata[8 + spslen + 1] = (ppslen >> 8) & 0x00ff;
              outputStream->codecpar->extradata[8 + spslen + 2] = ppslen & 0x00ff;
              for (i=0; i<ppslen; i++) {
               outputStream->codecpar->extradata[8 + spslen + 1 + 2 + i] = pps[i];
              }
              
              //Print out the full extradata
              /*for (size_t i = 0; i != outputStream->codecpar->extradata_size; ++i)
                    fprintf(stderr, "\\%02x", (unsigned char)outputStream->codecpar->extradata[i]);
                    fprintf(stderr,"\n");
              */
              
              if(avformat_write_header(outputFormatCtx, &fmt_opts) < 0){
                fprintf(stderr,"Error avformat_write_header");
                ret = 1;
              } else {
                extradata_written=true;
                fprintf(stderr,"EXTRADATA written\n");
		      }    
		   } 
		      
		   
       
		   
               
	       return;
	    }
	}  else {//extradata is written already
	    
	    
	    /*fprintf(stderr,"ORIGINAL \n");
	    for (size_t i = 0; i != buff->length; ++i)
                    fprintf(stderr, "\\%02x", (unsigned char)buff->data[i]);
                    fprintf(stderr,"\n");
	    */
	    
	    
	    opkt.data=(uint8_t*)av_mallocz(buff->length); //opkt.data will hold the 4 byte nal size and the actual bytestream
	    
	    //copy the nal size which will be 4 bytes less since the leading 4 bytes will be the nal size
	    uint32_t nal_size=buff->length-4; //little endian
	    
	    //write as big endian on the start of opkt.data
	    uint32_t b0,b1,b2,b3;

        b0 = (nal_size & 0x000000ff) << 24u;
        b1 = (nal_size & 0x0000ff00) << 8u;
        b2 = (nal_size & 0x00ff0000) >> 8u;
        b3 = (nal_size & 0xff000000) >> 24u;
        
        uint32_t res = b0 | b1 | b2 | b3;
        memcpy(opkt.data,&res,4);
        
        //Write the actual h264 data after the nal size
        memcpy(opkt.data+4,buff->data+4,buff->length-4);
        opkt.size=buff->length;
        
        
        /*
	    fprintf(stderr,"MODIFIED \n");
	    for (size_t i = 0; i != opkt.size; ++i)
                    fprintf(stderr, "\\%02x", (unsigned char)opkt.data[i]);
                    fprintf(stderr,"\n");
        */ 
        

	    AVRational time_base = outputStream->time_base;
        opkt.pts = opkt.dts = timeStampValue;
        opkt.pts = av_rescale_q(opkt.pts, AVRational{1,30}, time_base);
        opkt.dts = av_rescale_q(opkt.dts, AVRational{1,30}, time_base);
        opkt.duration = av_rescale_q(1, AVRational{1,30}, time_base);
        ret = av_write_frame(outputFormatCtx, &opkt);
        if(ret < 0)
        	fprintf(stderr, "Error writing frame\n");
        else
            fprintf(stderr, "FFMPEG Successful WRITE \n ");
            
    }        
}	

void ffmpeg_camera::Close_MP4(){
	ret = av_write_trailer(outputFormatCtx);
    if(ret < 0 )
    	fprintf(stderr,"Error writing trailer");
}	

//This requires swscale conversion to RGB first
void ffmpeg_camera::Save_PPM(AVFrame *pFrame, int iFrame)
 {
     FILE *pFile;
     char szFilename[32];
     int  y;
     
     sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
		  pFrame->linesize, 0, video_dec_ctx->height,
		  pFrameRGB->data, pFrameRGB->linesize);

     // Open file
     //sprintf(szFilename, "frame%d.ppm", iFrame);
     sprintf(szFilename, "frame.ppm", iFrame); //FIXME, just need one frame for testing
     pFile=fopen(szFilename, "wb");
     if(pFile==NULL)
         return;

     // Write header
     fprintf(pFile, "P6\n%d %d\n255\n", video_dec_ctx->width, video_dec_ctx->height);

     // Write pixel data
       for(y=0; y<video_dec_ctx->height; y++)
         fwrite(pFrameRGB->data[0]+y*pFrameRGB->linesize[0], 1, video_dec_ctx->width*3, pFile);
          
          

     // Close file
     fclose(pFile);
 }



uint8_t ffmpeg_camera::Save_JPEG(AVFrame *pFrame, int FrameNo) {
    
    FILE *JPEGFile;
    char JPEGFName[256];

    AVPacket packet = { 0 };
    av_init_packet(&packet);
  

    int packetcomplete=0;
    int ret=0;
    
    while (!packetcomplete) {
		//fprintf(stderr, "Encoding");
		ret = avcodec_send_frame(jpegContext, pFrame); 
      
        if (ret < 0) {
            //std::cout << "Error sending frame " << std::endl;
            continue;
        }
    
        avcodec_receive_packet(jpegContext, &packet); 
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
             continue;
        } 
        if (ret<0) {
            //std::cout << "Error receiving packet" << std::endl;
            continue;
        }
    packetcomplete=1;
    }
    	
    //sprintf(JPEGFName, "frame%d.jpg", FrameNo);
    sprintf(JPEGFName, "frame.jpg", FrameNo);  //just need one frame for testing
    JPEGFile = fopen(JPEGFName, "wb");
    fwrite(packet.data, 1, packet.size, JPEGFile);
    fclose(JPEGFile);

    av_packet_unref(&packet);
    
    return 0;
}

ffmpeg_camera::ffmpeg_camera(const uint8_t ctype, const char *src_filename):ctype(ctype), src_filename(src_filename){
	initialize();
};


ffmpeg_camera::~ffmpeg_camera(){
	fprintf(stderr, "Destructing camera\n");
	/* flush cached frames */
	
	av_packet_unref(&pkt);
	av_packet_unref(&opkt);
    
    avformat_network_deinit();
    if (dec_ctx)
      avcodec_flush_buffers(dec_ctx);
    if (video_dec_ctx)
      avcodec_free_context(&video_dec_ctx);
    if (fmt_ctx)
      avformat_close_input(&fmt_ctx);
    if (frame)
      av_frame_free(&frame);
      
    //FIXME, only used for YUV420 to RGB24 conversion for writing PPM files  
    if (pFrameRGB)
      av_frame_free(&pFrameRGB);
    if (rbuffer)
      free(rbuffer);
    if (sws_ctx)
    sws_freeContext(sws_ctx);
      
    //FIXME, only used for JPEG writing
    if (jpegContext)
    avcodec_free_context(&jpegContext);  
    if (sps)
       free(sps);
    if (pps)
       free(pps);
      
};

uint8_t ffmpeg_camera::decode_packet(const AVPacket *ipkt){
    int framecomplete=0;
    int ret=0;
    
    while (!framecomplete) {
        ret = avcodec_send_packet(video_dec_ctx, ipkt);
        if (ret < 0) {
            //std::cout << "Error sending packet " << std::endl;
            continue;
        }
    
        ret = avcodec_receive_frame(video_dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
             continue;
        } 
        if (ret<0) {
            //std::cout << "Error receiving frame" << std::endl;
            continue;
        }
    framecomplete=1;
    return 0;
    }	
    return -1;
	
};	

AVFrame * ffmpeg_camera::run(){
	while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_stream_idx) {
            int ret = decode_packet(&pkt);
            if (ret < 0 )
                continue;
            else
               fprintf(stderr, "FFMPEG Successful decode --> ");
            
            
        }//got a frame here
        return frame;    
        
    }
    return NULL;
};
