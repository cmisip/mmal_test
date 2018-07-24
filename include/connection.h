

//#ifndef mmal_engine
//#define mmal_engine

#include "mmal_engine.h"

//#endif
#include "interface/mmal/util/mmal_connection.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_util_params.h"

class Connection {


public:
Connection(mmal_engine *engine1, mmal_engine *engine2);
~Connection();
static void input_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
static void output_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
uint8_t enable();
static uint8_t connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection);
uint8_t run(AVFrame **frame, Buffer *outbuf);

uint32_t buffsize=0;
uint16_t width=0;
uint16_t height=0;

private:

mmal_engine *input_engine = NULL;
mmal_engine *output_engine = NULL;

MMAL_PORT_T *input_port=NULL;
MMAL_PORT_T *output_port=NULL;


MMAL_POOL_T *input_pool=NULL; 
MMAL_POOL_T *output_pool=NULL;   



/** Context for our application */
struct CONTEXT_T {
      MMAL_QUEUE_T *queue=0;
    } contexti, contexto;
    
MMAL_CONNECTION_T *connection = 0;
MMAL_STATUS_T status = MMAL_EINVAL;

};
