

#include "bcm_host.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/vcos/vcos.h"

#include <stdio.h>

#ifndef ffmpeg
#define ffmpeg

#include "ffmpeg_camera.h"

#endif

#ifndef obuffer
#define obuffer

#include "buffer.h"

#endif



class mmal_engine{
	
	
public:	
    //uint8_t init(const char **name);
    uint8_t set_video_input_port(uint16_t width, uint16_t height, MMAL_FOURCC_T iformat);
    
    uint8_t set_video_output_port(uint16_t width, uint16_t height, MMAL_FOURCC_T oformat);
    uint8_t set_input_flag(uint32_t name);
    uint8_t set_output_flag(uint32_t name);
    uint8_t enable_video_input_port();
    uint8_t enable_video_output_port();
    uint8_t create_input_pool();
    uint8_t create_output_pool();
    static void input_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer );
    static void output_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer );
    uint8_t enable();
    
   

	mmal_engine(const char *name);
	uint8_t run(AVFrame **frame, Buffer *outbuffer);
	~mmal_engine();


    
MMAL_COMPONENT_T *engine = NULL;
MMAL_POOL_T *pool_in = NULL, *pool_out = NULL;
MMAL_STATUS_T status = MMAL_EINVAL;
MMAL_DISPLAYREGION_T param;

MMAL_PORT_T *input_port=NULL;
MMAL_PORT_T *output_port=NULL;
const char * name;
uint32_t buffsize=0;

struct CONTEXT_T {
   //VCOS_SEMAPHORE_T semaphore;
   MMAL_QUEUE_T *queue=NULL;
} context;

uint16_t width=0;
uint16_t height=0; 



private:







//FUNCTIONS
#define CHECK_STATUS(status, msg) if (status != MMAL_SUCCESS) { fprintf(stderr, msg"\n"); }
	

	
};	
