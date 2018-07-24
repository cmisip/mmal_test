#ifndef ffmpeg
#define ffmpeg

#include "ffmpeg_camera.h"

#endif

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
        dec_ctx->pix_fmt=AV_PIX_FMT_YUV420P;

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

	
}
return ret;

};

	

ffmpeg_camera::ffmpeg_camera(const uint8_t ctype, const char *src_filename):ctype(ctype), src_filename(src_filename){
	initialize();
};


ffmpeg_camera::~ffmpeg_camera(){
	fprintf(stderr, "Destructing camera\n");
	/* flush cached frames */
    
    avformat_network_deinit();
    if (dec_ctx)
      avcodec_flush_buffers(dec_ctx);
    if (video_dec_ctx)
      avcodec_free_context(&video_dec_ctx);
    if (fmt_ctx)
      avformat_close_input(&fmt_ctx);
    if (frame)
      av_frame_free(&frame);
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
            //std::cout << "Error receiving packet" << std::endl;
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
};
