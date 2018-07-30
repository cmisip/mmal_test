#ifndef videowriter
#define videowriter

#include "videowriter.h"

#endif
#ifndef mmal
#define mmal
#include "bcm_host.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/vcos/vcos.h"
#endif



Encoder::Encoder(int width,int height,const char* target){
  int err;
  AVOutputFormat  *fmt;
  AVCodec         *codec;
  AVDictionary *fmt_opts = NULL;
  av_register_all();
  this->fmt_ctx = avformat_alloc_context();
  //if (this->fmt_ctx == NULL) {
   // throw AVException(ENOMEM,"can not alloc av context");
  //}
  //init encoding format
  fprintf(stderr, "HOLA");
  fmt = av_guess_format(NULL, target, NULL);
  if (!fmt) {
    std::cout<<"Could not deduce output format from file extension: using MPEG4."<<std::endl;
    fmt = av_guess_format("mpeg4", NULL, NULL);
  }
  //std::cout <<fmt->long_name<<std::endl;
  //Set format header infos
  fmt_ctx->oformat = fmt;
  snprintf(fmt_ctx->filename, sizeof(fmt_ctx->filename), "%s", target);
  
  
  //Reference for AvFormatContext options : https://ffmpeg.org/doxygen/2.8/movenc_8c_source.html
  //Set format's privater options, to be passed to avformat_write_header()
  err = av_dict_set(&fmt_opts, "movflags", "faststart", 0);
  if (err < 0) {
    fprintf(stderr,"av_dict_set movflags");
  }
  //default brand is "isom", which fails on some devices
  av_dict_set(&fmt_opts, "brand", "mp42", 0);
  if (err < 0) {
    fprintf(stderr,"av_dict_set brand");
  }
  
  
  codec = avcodec_find_encoder(OUTPUT_CODEC);
   
  //codec = avcodec_find_encoder(fmt->video_codec);
  /*if (!codec) {
    throw AVException(1,"can't find encoder");
  }*/
  if (!(st = avformat_new_stream(fmt_ctx, codec))) {
    //throw AVException(1,"can't create new stream");
    fprintf(stderr,"avformat");
  }
  //set stream time_base
  /* frames per second FIXME use input fps? */
  fprintf(stderr, "STOP");
  st->time_base = (AVRational){ 1, 25};
  fprintf(stderr, "THERER");
  //Set codec_ctx to stream's codec structure
  codec_ctx = st->codec;
  /* put sample parameters */
  codec_ctx->sample_fmt     = codec->sample_fmts           ?  codec->sample_fmts[0]           : AV_SAMPLE_FMT_S16;
  codec_ctx->width = width;
  codec_ctx->height = height;
  codec_ctx->time_base = st->time_base;
  codec_ctx->pix_fmt = OUTPUT_PIX_FMT;
  /* Apparently it's in the example in master but does not work in V11
  if (o_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
      codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  */
  //H.264 specific options
  codec_ctx->gop_size = 25;
  codec_ctx->level = 31;
  
  err = av_opt_set(codec_ctx->priv_data, "crf", "12", 0);
  if (err < 0) {
    fprintf(stderr,"av_opt_set crf");
  }
  err = av_opt_set(codec_ctx->priv_data, "profile", "main", 0);
  if (err < 0) {
    fprintf(stderr,"av_opt_set profile");
  }
  err = av_opt_set(codec_ctx->priv_data, "preset", "slow", 0);
  if (err < 0) {
    fprintf(stderr,"av_opt_set preset");
  }
  
 
  // disable b-pyramid. CLI options for this is "-b-pyramid 0"
  //Because Quicktime (ie. iOS) doesn't support this option
  err = av_opt_set(codec_ctx->priv_data, "b-pyramid", "0", 0);
  if (err < 0) {
    fprintf(stderr,"av_opt_set b-pyramid");
  }
  //It's necessary to open stream codec to link it to "codec" (the encoder).
  err = avcodec_open2(codec_ctx, codec, NULL);
  if (err < 0) {
    throw err;
  }

  //* dump av format informations
  av_dump_format(fmt_ctx, 0, target, 1);
  //*/
  err = avio_open(&fmt_ctx->pb, target,AVIO_FLAG_WRITE);
  if(err < 0){
    throw err;
  }

  //Write file header if necessary
  err = avformat_write_header(fmt_ctx,&fmt_opts);
  if(err < 0){
    throw err;
  }

  /* Alloc tmp frame once and for all*/
  tmp_frame = av_frame_alloc();
  if(!tmp_frame){
    throw ENOMEM;
  }
  //Make sure the encoder doesn't keep ref to this frame as we'll modify it.
  av_frame_make_writable(tmp_frame);
  tmp_frame->format = codec_ctx->pix_fmt;
  tmp_frame->width  = codec_ctx->width;
  tmp_frame->height = codec_ctx->height;
  err = av_image_alloc(tmp_frame->data, tmp_frame->linesize, codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, 32);
  if (err < 0) {
    throw ENOMEM;
  }

}

Encoder::~Encoder(){
  int err;
  std::cout<<"cleaning Encoder"<<std::endl;
  //Write pending packets
  //while((err = write((AVFrame*)NULL)) == 1){};
  //if(err < 0 ){
  //  std::cout <<"error writing delayed frame"<<std::endl;
  //}
  //Write file trailer before exit
  av_write_trailer(this->fmt_ctx);
  //close file
  avio_close(fmt_ctx->pb);
  avformat_free_context(this->fmt_ctx);
  //avcodec_free_context(&this->codec_ctx);

  av_freep(&this->tmp_frame->data[0]);
  av_frame_free(&this->tmp_frame);
}

//Return 1 if a packet was written. 0 if nothing was done.
// return error_code < 0 if there was an error.


int Encoder::write(Buffer *buff){
  int err;
  int got_output = 1;
  AVPacket pkt = {0};
  //av_init_packet(&pkt);
  
  //AVPacket pkt;
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
        
        
    //    av_interleaved_write_frame(ofctx, &pkt);
      //  av_packet_unref(&pkt);
    //}
  

  //Set frame pts, monotonically increasing, starting from 0
  //if(frame != NULL) frame->pts = pts++; //we use frame == NULL to write delayed packets in destructor
  //err = avcodec_encode_video2(this->codec_ctx, &pkt, frame, &got_output);
  //if (err < 0) {
    //std::cout <<AVException(err,"encode frame");
  //  return err;
  //}
 // if(got_output){
    av_packet_rescale_ts(&pkt, this->codec_ctx->time_base, this->st->time_base);
    pkt.stream_index = this->st->index;
    /* write the frame */
    //printf("Write packet %03d of size : %05d\n",pkt.pts,pkt.size);
    //write_frame will take care of freeing the packet.
    err = av_interleaved_write_frame(this->fmt_ctx,&pkt);
    if(err < 0){
      //std::cout <<AVException(err,"write frame");
      av_packet_unref(&pkt);
      return err;
    }
    av_packet_unref(&pkt);
    //return 1;
  //}else{
    return 0;
  //}
}
