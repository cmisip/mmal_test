#include <stdio.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string.h>
#include <algorithm>
#include <string> 

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavcodec/avfft.h>

#include <libavdevice/avdevice.h>

#include <libavfilter/avfilter.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>

#include <libavformat/avformat.h>
#include <libavformat/avio.h>

    // libav resample

#include <libavutil/opt.h>
#include <libavutil/common.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/file.h>


    // hwaccel
#include "libavcodec/vdpau.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_vdpau.h"

    // lib swresample

#include <libswscale/swscale.h>
}
    


#ifndef obuffer
#define obuffer

#include "buffer.h"

#endif

    

    class VideoCapture {
    public:

        VideoCapture(int iwidth, int iheight, int ifpsrate, int ibitrate);
        
        

        ~VideoCapture() {
            Free();
        }

        //static std::ofstream logFile;


        void Init();

        //void AddFrame(uint8_t **data, int length);
        void AddFrame(Buffer *buff);

        void Finish();
        
        typedef void(*FuncPtr)(const char *);
        
        
        static void avlog_cb(void *, int level, const char * fmt, va_list vargs);
        
        //VideoCapture* Init(int width, int height, int fps, int bitrate); 

        //void AddFrame(uint8_t *data, int length, VideoCapture *vc);
   

        void Finish(VideoCapture *vc);
    

        void SetDebug(FuncPtr fp); 

    private:
        
        AVOutputFormat *oformat;
        AVFormatContext *ofctx;

        AVStream *videoStream;
        AVFrame *videoFrame;

        AVCodec *codec;
        AVCodecContext *cctx;

        SwsContext *swsCtx;

        int frameCounter;

        int fps;
        int width;
        int height;
        int bitrate;

        void Free();

        int16_t Remux();
        
        
        static void Log(std::string str);
        void Debug(std::string str, int err);
        
       
        FuncPtr ExtDebug;
        char errbuf[32];
        
        
    };

    

