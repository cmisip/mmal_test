#ifndef videocapture
#define videocapture


#include "videocapture.h"

#endif


#ifndef mmal
#define mmal

#include "bcm_host.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/vcos/vcos.h"

#endif




#define VIDEO_TMP_FILE "tmp.h264"
#define FINAL_FILE_NAME "record.mp4"


using namespace std;

void VideoCapture::Log(std::string str) {
        //logFile.open("Logs.txt", std::ofstream::app);
        //logFile.write(str.c_str(), str.size());
        //logFile.close();
    }


void VideoCapture::Debug(std::string str, int err) {
        Log(str + " " + std::to_string(err));
        if (err < 0) {
            av_strerror(err, errbuf, sizeof(errbuf));
            str += errbuf;
        }
        Log(str);
        ExtDebug(str.c_str());
    }
    





void VideoCapture::avlog_cb(void *, int level, const char * fmt, va_list vargs) {
        static char message[8192];
        //vsnprintf_s(message, sizeof(message), fmt, vargs);
        Log(message);
    }
    
/*VideoCapture* VideoCapture::Init(int width, int height, int fps, int bitrate) {
        VideoCapture *vc = new VideoCapture();
        vc->Init(width, height, fps, bitrate);
        return vc;
    };    
*/    
/*void VideoCapture::AddFrame(uint8_t *data, int length, VideoCapture *vc) {
        vc->AddFrame(data, length);
    }*/

void VideoCapture::Finish(VideoCapture *vc) {
        vc->Finish();
    }

void VideoCapture::SetDebug(FuncPtr fp) {
        ExtDebug = fp;
    };    

VideoCapture::VideoCapture(int iwidth, int iheight, int ifpsrate, int ibitrate):
      width(iwidth),height(iheight),fps(ifpsrate),bitrate(ibitrate){
	        oformat = NULL;
            ofctx = NULL;
            videoStream = NULL;
            videoFrame = NULL;
            swsCtx = NULL;
            frameCounter = 0;

            // Initialize libavcodec
            av_register_all();
            av_log_set_callback(avlog_cb);
}

void VideoCapture::Init() {

    //fps = fpsrate;

    int err;

    if (!(oformat = av_guess_format(NULL, VIDEO_TMP_FILE, NULL))) {
        Debug("Failed to define output format\n", 0);
        //return;
    } else
     fprintf(stderr,"format %s \n", oformat->long_name);

    
        
    if ((err = avformat_alloc_output_context2(&ofctx, oformat, NULL, VIDEO_TMP_FILE) < 0)) {
        fprintf(stderr,"Failed to allocate output context\n");
        //Free();
        //return;
    }
   
    if (!ofctx)
      fprintf(stderr, "ERROR");
      
      
    //fprintf(stderr,"input format %s \n",ofctx->iformat->long_name);
    fprintf(stderr,"output format %s \n",ofctx->oformat->long_name);

    if (!(codec = avcodec_find_encoder(oformat->video_codec))) {
        fprintf(stderr,"Failed to find encoder\n");
        //Free();
        //return;
    }

    if (!(videoStream = avformat_new_stream(ofctx, codec))) {
        fprintf(stderr,"Failed to create new stream\n");
        //Free();
        //return;
    }

    if (!(cctx = avcodec_alloc_context3(codec))) {
        fprintf(stderr,"Failed to allocate codec context \n");
        //Free();
        //return;
    }
    
    videoStream->codecpar->codec_id = oformat->video_codec;
    videoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    videoStream->codecpar->width = width;
    videoStream->codecpar->height = height;
    videoStream->codecpar->format = AV_PIX_FMT_YUV420P;
    videoStream->codecpar->bit_rate = bitrate * 1000;
    videoStream->time_base = { 1, fps };

    avcodec_parameters_to_context(cctx, videoStream->codecpar);
    cctx->time_base = { 1, fps };
    cctx->max_b_frames = 2;
    cctx->gop_size = 12;
    if (videoStream->codecpar->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(cctx, "preset", "ultrafast", 0);
    }
    if (ofctx->oformat->flags & AVFMT_GLOBALHEADER) {
        cctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    avcodec_parameters_from_context(videoStream->codecpar, cctx);

    if ((err = avcodec_open2(cctx, codec, NULL)) < 0) {
        fprintf(stderr,"Failed to open codec\n");
        //Free();
        //return;
    }

    

    if (!(oformat->flags & AVFMT_NOFILE)) {
        if ((err = avio_open(&ofctx->pb, VIDEO_TMP_FILE, AVIO_FLAG_WRITE)) < 0) {
            fprintf(stderr,"Failed to open file\n");
            //Free();
            //return;
        }
    }

    if ((err = avformat_write_header(ofctx, NULL)) < 0) {
        fprintf(stderr,"Failed to write header\n");
        //Free();
        //return;
    } 
      
    
    av_dump_format(ofctx, 0, VIDEO_TMP_FILE, 1);
    
}

void VideoCapture::AddFrame(Buffer *buff) {
    int err;
    /*if (!videoFrame) {

        videoFrame = av_frame_alloc();
        videoFrame->format = AV_PIX_FMT_YUV420P;
        videoFrame->width = cctx->width;
        videoFrame->height = cctx->height;

        if ((err = av_frame_get_buffer(videoFrame, 32)) < 0) {
            Debug("Failed to allocate picture", err);
            return;
        }
    }

    if (!swsCtx) {
        swsCtx = sws_getContext(cctx->width, cctx->height, AV_PIX_FMT_RGB24, cctx->width, cctx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0);
    }

    int inLinesize[1] = { 3 * cctx->width };

    // From RGB to YUV
    sws_scale(swsCtx, (const uint8_t * const *)&data, inLinesize, 0, cctx->height, videoFrame->data, videoFrame->linesize);

    videoFrame->pts = frameCounter++;

    if ((err = avcodec_send_frame(cctx, videoFrame)) < 0) {
        Debug("Failed to send frame", err);
        return;
    }
     */
     
     
    AVPacket pkt;
    av_init_packet(&pkt);
    uint8_t *data=(uint8_t *)av_malloc(buff->length);
    av_packet_from_data(&pkt,data,buff->length);
    //pkt.data = NULL;
    pkt.size = buff->length;
    pkt.pts = buff->pts;
    pkt.dts = buff->dts;

    //if (avcodec_receive_packet(cctx, &pkt) == 0) {
    if (buff->flags & MMAL_BUFFER_HEADER_FLAG_KEYFRAME)
        pkt.flags |= AV_PKT_FLAG_KEY;
        
        memcpy(pkt.data,buff->data, buff->length);
        
        
        av_interleaved_write_frame(ofctx, &pkt);
        av_packet_unref(&pkt);
    //}
}

void VideoCapture::Finish() {
    //DELAYED FRAMES
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    for (;;) {
        avcodec_send_frame(cctx, NULL);
        if (avcodec_receive_packet(cctx, &pkt) == 0) {
            av_interleaved_write_frame(ofctx, &pkt);
            av_packet_unref(&pkt);
        }
        else {
            break;
        }
    }

    av_write_trailer(ofctx);
    if (!(oformat->flags & AVFMT_NOFILE)) {
        int err = avio_close(ofctx->pb);
        if (err < 0) {
            fprintf(stderr,"Failed to close file");
        }
    }

    Free();
    
    

    Remux();
   
}

void VideoCapture::Free() {
    if (videoFrame) {
        av_frame_free(&videoFrame);
    }
    if (cctx) {
        avcodec_free_context(&cctx);
    }
    if (ofctx) {
        avformat_free_context(ofctx);
    }
    if (swsCtx) {
        sws_freeContext(swsCtx);
    }
}

int16_t VideoCapture::Remux() {
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVDictionary *fmt_opts = NULL;
    int err;
    int ts = 0;

    if (avformat_open_input(&ifmt_ctx, VIDEO_TMP_FILE, 0, 0) < 0) {
        fprintf(stderr,"Failed to open input file for remuxing");
        return err;
    }
        
    
    if ((avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr,"Failed to retrieve input stream information");
                
        //if (ifmt_ctx) {
        //  avformat_close_input(&ifmt_ctx);
        //}
        return err;
    }
    
    AVOutputFormat * outFmt = av_guess_format("mp4", NULL, NULL);
    
     fprintf(stderr,"format %s \n", outFmt->long_name);
    if (avformat_alloc_output_context2(&ofmt_ctx, outFmt, NULL, NULL)<0) {
        fprintf(stderr,"Failed to allocate output context");
        //if (ofmt_ctx) {
       //   avformat_free_context(ofmt_ctx);
       // }
        
        return err;
    }
    
     fprintf(stderr,"output mp4 format %s \n",ofmt_ctx->oformat->long_name);

     
    AVStream *inVideoStream = ifmt_ctx->streams[0];
    AVStream *outVideoStream = avformat_new_stream(ofmt_ctx, 0);
    
    if (!outVideoStream) {
        fprintf(stderr,"Failed to allocate output video stream");
        if (ifmt_ctx) {
          avformat_close_input(&ifmt_ctx);
        }  
        if (ofmt_ctx) {
          avformat_free_context(ofmt_ctx);
        }  
        return err;
        
    }
    
    AVCodec * codec = NULL;
    avcodec_get_context_defaults3(outVideoStream->codec, codec);
    outVideoStream->codec->thread_count = 1;
    
#if (LIBAVFORMAT_VERSION_MAJOR == 53)
    outStrm->stream_copy = 1;
#endif

    outVideoStream->codec->coder_type = AVMEDIA_TYPE_VIDEO;
    if(ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) 
        outVideoStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

    outVideoStream->codec->sample_aspect_ratio = outVideoStream->sample_aspect_ratio = inVideoStream->sample_aspect_ratio; 
    
#if (LIBAVFORMAT_VERSION_MAJOR == 53)
    ofmt_ctx->timestamp = 0;
#endif
    
    /*AVCodecContext *pcc = outVideoStream->codec;
    avcodec_get_context_defaults3( pcc, NULL );
    pcc->codec_type = AVMEDIA_TYPE_VIDEO;

//CodecID codec_id = CODEC_ID_H264;
    pcc->codec_id = AV_CODEC_ID_H264;
    pcc->bit_rate = bitrate;
    pcc->width = width;
    pcc->height = height;
    pcc->time_base.num = 1;
    pcc->time_base.den = fps;
    fprintf(stderr,"HOLAAAAA");
    
    outVideoStream->time_base = { 1, fps };
    avcodec_parameters_copy(outVideoStream->codecpar, inVideoStream->codecpar);
    outVideoStream->codecpar->codec_tag = 0;
    
    //if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
    //    outVideoStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    //}
    
    av_set_parameters( ofmt_ctx, 0 );
	*/
      
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if ((err = avio_open(&ofmt_ctx->pb, FINAL_FILE_NAME, AVIO_FLAG_READ_WRITE)) < 0) {
            fprintf(stderr,"Failed to open output file ");
            goto end;
        }
    }
    
#if (LIBAVFORMAT_VERSION_MAJOR == 53)
    AVFormatParameters params = {0};
    err = av_set_parameters(ofmt_ctx, &params);
    if (err < 0)
        goto end;
#endif
    
    av_dump_format(ofmt_ctx, 0, FINAL_FILE_NAME, 1);
    
    //ofmt_ctx->streams[0]->codec=0;
    //outVideoStream->codecpar->codec_tag = 0;
    
    if (!outVideoStream->codec->codec_tag)
    {
        if (! ofmt_ctx->oformat->codec_tag
            || av_codec_get_id (ofmt_ctx->oformat->codec_tag, inVideoStream->codec->codec_tag) == outVideoStream->codec->codec_id
            || av_codec_get_tag(ofmt_ctx->oformat->codec_tag, inVideoStream->codec->codec_id) <= 0)
                    outVideoStream->codec->codec_tag = inVideoStream->codec->codec_tag;
    }
    
    //ofmt_ctx->oformat->codec_tag=0;
    av_dict_set(&fmt_opts, "brand", "mp42", 0);
    
#if LIBAVFORMAT_VERSION_MAJOR == 53
if ((err = avformat_write_header(ofmt_ctx)) <0 ){
    //av_write_header(ofmt_ctx);
#else
    if ((err = avformat_write_header(ofmt_ctx, NULL)) <0 ){
		char errs[200];
        av_make_error_string(errs, 200, err);
        printf("Write header %d: %s\n", err, errs);
	}	
#endif


av_dict_set(&fmt_opts, "brand", "mp42", 0);
    
   //if ((err = avformat_write_header(ofmt_ctx, &fmt_opts)) < 0) {
     //   fprintf(stderr,"Failed to write header to outputt file %d ",err);
        //av_strerror	(err, errbuf, 30 ) 
    //    char output[20];
      //  av_strerror(err,output,20);
        //fprintf(stderr,output);
        
	//char errs[200];
    //av_make_error_string(errs, 200, err);
   // printf("Write header %d: %s\n", err, errs);
		
        
    //    goto end;
    //}
    
    fprintf(stderr,"HELOOOOOO");
    AVPacket videoPkt;
    av_init_packet(&videoPkt);
    videoPkt.data = NULL;
    videoPkt.size = 0;
    fprintf(stderr,"videopackt");
    
    
    while (true) {
		fprintf(stderr, "TRUE");
        if ((err = av_read_frame(ifmt_ctx, &videoPkt)) < 0) {
			fprintf(stderr,"readframe");
            break;
        } else
            fprintf(stderr,"Read the frame");
        fprintf(stderr,"break");
        videoPkt.stream_index = outVideoStream->index;
        fprintf(stderr,"break1");
        videoPkt.pts = ts;
        fprintf(stderr,"break2");
        videoPkt.dts = ts;
        fprintf(stderr,"break3");
        videoPkt.duration = av_rescale_q(videoPkt.duration, inVideoStream->time_base, outVideoStream->time_base);
        fprintf(stderr,"break4");
        ts += videoPkt.duration;
        fprintf(stderr,"break5");
        videoPkt.pos = -1;
        fprintf(stderr,"break6");
        if (!ofmt_ctx)
         fprintf(stderr,"NONE");
         if (!&videoPkt)
         fprintf(stderr,"NOPKT");
       //if ((err = av_interleaved_write_frame(ofmt_ctx, &videoPkt)) < 0) {
		  // if ((err = av_write_frame(ofmt_ctx, &videoPkt)) < 0) {
         //   fprintf(stderr,"Failed to mux packet");
         //   av_packet_unref(&videoPkt);
         //   break;
       // }
        av_packet_unref(&videoPkt);
        fprintf(stderr,"break7");
    }

    //av_write_trailer(ofmt_ctx);

end:
    //if (ifmt_ctx) {
    //    avformat_close_input(&ifmt_ctx);
    //}
    //if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
    //    avio_closep(&ofmt_ctx->pb);
    //}
    //if (ofmt_ctx) {
    //    avformat_free_context(ofmt_ctx);
    //}
    fprintf(stderr,"END");
}
