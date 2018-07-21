
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/motion_vector.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include "libswscale/swscale.h"
#include <libavutil/frame.h>
}

#include <stdio.h>
#include <sys/types.h>

class ffmpeg_camera {

public:
uint8_t ctype = 0;
uint16_t width=0, height=0;
//uint32_t bufsize=0;


ffmpeg_camera(const uint8_t type, const char *src_filename);
~ffmpeg_camera();
AVFrame *  run();
uint8_t decode_packet(const AVPacket *ipkt);

private:
  //0 is software, 1 is hardware
AVDictionary *opts = NULL;
int ret;
AVStream *st;
const char *src_filename = NULL;
AVFormatContext *fmt_ctx = NULL;
AVCodecContext *video_dec_ctx = NULL;
AVStream *video_stream = NULL;
AVCodec *dec = NULL;
AVCodecContext *dec_ctx = NULL;

int video_stream_idx = -1;
AVFrame *frame = NULL;
AVPacket pkt = { 0 };



//FUNCTIONS

uint8_t initialize();


};
