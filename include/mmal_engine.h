

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



class mmal_engine{
	
public:	
    //uint8_t init(const char **name);
    uint8_t set_input_port(uint16_t width, uint16_t height, MMAL_FOURCC_T iformat);
    uint8_t set_output_port(uint16_t width, uint16_t height, MMAL_FOURCC_T oformat);
    uint8_t set_input_flag(uint32_t name);
    uint8_t set_output_flag(uint32_t name);
    static void input_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer );
    static void output_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer );
    uint8_t enable();

	mmal_engine(const char *name);
	uint8_t run(AVFrame **frame, uint8_t **outbuffer, uint32_t outbuf_size);
	~mmal_engine();


private:

MMAL_STATUS_T status = MMAL_EINVAL;
MMAL_COMPONENT_T *engine = NULL;
MMAL_POOL_T *pool_in = NULL, *pool_out = NULL;

const char * name;
uint32_t buffsize=0;

struct CONTEXT_T {
   //VCOS_SEMAPHORE_T semaphore;
   MMAL_QUEUE_T *queue=NULL;
} context;

uint16_t width=0;
uint16_t height=0;



//FUNCTIONS
#define CHECK_STATUS(status, msg) if (status != MMAL_SUCCESS) { fprintf(stderr, msg"\n"); }
	

	
};	