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
        outputStream->codecpar->width = st->codecpar->width;
        outputStream->codecpar->height = st->codecpar->height;
        outputStream->codecpar->format = AV_PIX_FMT_YUV420P;
        outputStream->codecpar->bit_rate = 4000000;
        outputStream->time_base = AVRational{1,30};
        
        
        if ( !(outputFormatCtx->flags & AVFMT_NOFILE) )
          if( avio_open2(&outputFormatCtx->pb , filename , AVIO_FLAG_WRITE ,NULL, NULL) < 0 ){
            fprintf(stderr,"Error avio_open2");
            ret = 1;
          }

        if(avformat_write_header(outputFormatCtx , NULL) < 0){
            fprintf(stderr,"Error avformat_write_header");
            ret = 1;
        }
    
        fprintf(stderr,"OUTPUT FORMAT-------------------------------");
        av_dump_format(outputFormatCtx , 0 ,filename ,1);
        
        av_init_packet(&pkt);
        	
}	


void ffmpeg_camera::Save_MP4(Buffer *buff, int timeStampValue){
	    opkt.data=(uint8_t* )av_mallocz(buff->length);
        memcpy(opkt.data,buff->data,buff->length);
        opkt.size=buff->length;
        

	    AVRational time_base = outputStream->time_base;
        opkt.pts = opkt.dts = timeStampValue;
        opkt.pts = av_rescale_q(opkt.pts, AVRational{1,30}, time_base);
        opkt.dts = av_rescale_q(opkt.dts, AVRational{1,30}, time_base);
        opkt.duration = av_rescale_q(1, AVRational{1,30}, time_base);
        ret = av_write_frame(outputFormatCtx, &opkt);
        if(ret < 0)
        	fprintf(stderr, "Error write frame");
        else
            fprintf(stderr, "FFMPEG Successful WRITE --> ");
}	

void ffmpeg_camera::Close_MP4(){
	ret = av_write_trailer(outputFormatCtx);
    if(ret < 0 )
    	fprintf(stderr,"Error write trailer");
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
