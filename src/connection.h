

//#ifndef mmal_engine
//#define mmal_engine

#include "mmal_engine.h"

//#endif
#include "interface/mmal/util/mmal_connection.h"

class Connection {


public:
Connection(mmal_engine *engine1, mmal_engine *engine2);
~Connection();
static void control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
static void connection_callback(MMAL_CONNECTION_T *connection);
uint8_t enable();
uint8_t run(AVFrame **frame, Buffer *outbuf);
private:

mmal_engine *input_engine = NULL;
mmal_engine *output_engine = NULL;

/** Context for our application */
struct CONTEXT_T {
   //VCOS_SEMAPHORE_T semaphore;
   MMAL_STATUS_T status;
   MMAL_BOOL_T eos;
} context;

MMAL_CONNECTION_T *connection = 0;
MMAL_STATUS_T status = MMAL_EINVAL;

};
