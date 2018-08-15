
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/motion_vector.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include "libswscale/swscale.h"
#include <libavutil/frame.h>
#include "libavformat/avformat.h"
#include <libswscale/swscale.h>
}

#include <stdio.h>
#include <sys/types.h>
#include <string>

#ifndef obuffer
#define obuffer

#include "buffer.h"

#endif

#define MMAL_BUFFER_HEADER_FLAG_CONFIG                 (1<<5)

class extradata_pack {
	public:
	uint8_t size=0;
	uint8_t *data=NULL;
	void init(uint8_t size, uint8_t** buffer);
	void print();
	~extradata_pack();
};	

class ffmpeg_camera {

public:
uint8_t ctype = 0;
uint16_t width=0, height=0;
bool extradata_written=0;

uint8_t nal_unit_type=0;

extradata_pack edata;

	


ffmpeg_camera(const uint8_t type, const char *src_filename);
~ffmpeg_camera();
AVFrame *  run();
AVPacket * get();
uint8_t decode_packet(const AVPacket *ipkt);
uint8_t save_frame_as_jpeg(AVFrame *pFrame, int FrameNo);
void Save_PPM(AVFrame *pFrame, int iFrame);
uint8_t Save_JPEG(AVFrame *pFrame, int iFrame);
uint16_t get_width();
uint16_t get_height();
extradata_pack *get_extradata();


void Init_MP4(const char* filename);
void Save_MP4(Buffer *buff, int timeStampValue);
void Close_MP4();



private:
  
AVDictionary *opts = NULL;
int ret;
AVStream *st;
const char *src_filename = NULL;
AVFormatContext *fmt_ctx = NULL;
AVCodecContext *video_dec_ctx = NULL;
AVStream *video_stream = NULL;
AVCodec *dec = NULL;
AVCodecContext *dec_ctx = NULL;
AVCodec* outCodec;
AVStream* outputStream;
uint8_t *sps=NULL;
uint8_t *pps=NULL;
uint16_t spslen=0;
uint16_t ppslen=0;


//FIXME, START : Only used to test if libavcodec is capturing frames properly
struct SwsContext *sws_ctx = NULL;
AVFrame           *pFrameRGB = NULL;
uint32_t          numBytes;
uint8_t           *rbuffer = NULL;
//FIXME, END

//FIXME, START : Only used to check libavcodec jpeg saving
AVCodecContext *jpegContext = NULL;
//AVFrame        *pFrameJPEG = NULL;
AVCodec *jpegCodec = NULL;
//FIXME, END :

AVOutputFormat *outputFormat;
AVFormatContext *outputFormatCtx;
AVDictionary *fmt_opts = NULL;

int video_stream_idx = -1;
AVFrame *frame = NULL;
AVPacket pkt = { 0 };
AVPacket opkt = { 0 };



//FUNCTIONS

uint8_t initialize();


};
