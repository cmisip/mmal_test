#include "connection.h"




/** Callback from a control port. Error and EOS events stop playback. */
void Connection::control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   struct CONTEXT_T *ctx = (struct CONTEXT_T *)port->userdata;

   if (buffer->cmd == MMAL_EVENT_ERROR)
      ctx->status = *(MMAL_STATUS_T *)buffer->data;
   else if (buffer->cmd == MMAL_EVENT_EOS)
      ctx->eos = MMAL_TRUE;

   mmal_buffer_header_release(buffer);

   /* The processing is done in our main thread */
   //vcos_semaphore_post(&ctx->semaphore);
}

/** Callback from the connection. Buffer is available. */
void Connection::connection_callback(MMAL_CONNECTION_T *connection)
{
   struct CONTEXT_T *ctx = (struct CONTEXT_T *)connection->user_data;

   /* The processing is done in our main thread */
   //vcos_semaphore_post(&ctx->semaphore);
}


uint8_t Connection::enable(){
	status = mmal_connection_enable(connection);
    CHECK_STATUS(status, "failed to enable connection");
};	


Connection::Connection(mmal_engine *engine1, mmal_engine *engine2){


status = mmal_connection_create(&connection, engine2->output, engine1->input, MMAL_CONNECTION_FLAG_TUNNELLING);
   CHECK_STATUS(status, "failed to create connection \n");

   connection->user_data = &context;
   connection->callback = connection_callback;

};

Connection::~Connection(){
	fprintf(stderr,"Destroying connection %s", connection->out->name);
	if (connection)
         mmal_connection_destroy(connection);
};
