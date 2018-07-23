//#ifndef connection
//#define connection

#include "connection.h"

//#endif





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
    
   MMAL_ES_FORMAT_T *format_in = input_engine->engine->input[0]->format; 
   /* Display the port format */
   fprintf(stderr,"---------------------------------------------------\n");
   fprintf(stderr, "INPUT %s\n", input_engine->engine->input[0]->name);
   fprintf(stderr, " type: %i, fourcc: %4.4s\n", format_in->type, (char *)&format_in->encoding);
   fprintf(stderr, " bitrate: %i, framed: %i\n", format_in->bitrate,
           !!(format_in->flags & MMAL_ES_FORMAT_FLAG_FRAMED));
   fprintf(stderr, " width: %i, height: %i, (%i,%i,%i,%i)\n",
           format_in->es->video.width, format_in->es->video.height,
           format_in->es->video.crop.x, format_in->es->video.crop.y,
           format_in->es->video.crop.width, format_in->es->video.crop.height);
           
            
    
   MMAL_ES_FORMAT_T *format_out = output_engine->engine->output[0]->format; 
    /* Display the port format */
   fprintf(stderr,"---------------------------------------------------\n");
   fprintf(stderr, "OUTPUT %s\n", output_engine->engine->output[0]->name);
   fprintf(stderr, " type: %i, fourcc: %4.4s\n", format_out->type, (char *)&format_out->encoding);
   fprintf(stderr, " bitrate: %i, framed: %i\n", format_out->bitrate,
           !!(format_out->flags & MMAL_ES_FORMAT_FLAG_FRAMED));
   fprintf(stderr, " width: %i, height: %i, (%i,%i,%i,%i)\n",
           format_out->es->video.width, format_out->es->video.height,
           format_out->es->video.crop.x, format_out->es->video.crop.y,
           format_out->es->video.crop.width, format_out->es->video.crop.height);
           
   
   
   
   
    return status;
};	


uint8_t Connection::run(AVFrame **frame, Buffer *outbuf){
	
	//Send frame buffer data to input_engine input port
	MMAL_BUFFER_HEADER_T *buffer;
	if ((buffer = mmal_queue_get(input_engine->pool_in->queue)) != NULL)
      {
         
         mmal_buffer_header_mem_lock(buffer);
         av_image_copy_to_buffer(buffer->data, input_engine->buffsize, (const uint8_t **)(*frame)->data, (*frame)->linesize,
                                AV_PIX_FMT_YUV420P, (*frame)->width, (*frame)->height, 1);
         buffer->length=input_engine->buffsize;
         mmal_buffer_header_mem_unlock(buffer);
         
            

         fprintf(stderr, "%s sending >>>>> %i bytes\n", input_engine->engine->input[0]->name, (int)buffer->length);
         status = mmal_port_send_buffer(input_engine->engine->input[0], buffer);
         CHECK_STATUS(status, "failed to send buffer");
      }
      
      //get buffers from the output_engine queue and process them
      
      while ((buffer = mmal_queue_get(output_engine->context.queue)) != NULL)
      {
         mmal_buffer_header_mem_lock(buffer);
         fprintf(stderr, "%s receiving %d bytes <<<<< frame\n", output_engine->engine->output[0]->name, buffer->length);
         
         memset(outbuf->data,0,outbuf->length);
         memcpy(outbuf->data,buffer->data,buffer->length);
         outbuf->length=buffer->length;
         outbuf->flags=buffer->flags;
         outbuf->cmd=buffer->cmd;
         outbuf->pts=buffer->pts;
         outbuf->dts=buffer->dts;
         
         mmal_buffer_header_mem_unlock(buffer); 
         mmal_buffer_header_release(buffer);
      }
      
      //send free buffers to the output_engine port
      while ((buffer = mmal_queue_get(output_engine->pool_out->queue)) != NULL)
      {
         status = mmal_port_send_buffer(output_engine->engine->output[0], buffer);
         CHECK_STATUS(status, "failed to send buffer");
      }
     return status;    
};	

Connection::Connection(mmal_engine *engine1, mmal_engine *engine2):input_engine(engine1),output_engine(engine2){


status = mmal_connection_create(&connection, engine1->engine->output[0], engine2->engine->input[0], MMAL_CONNECTION_FLAG_TUNNELLING);
   CHECK_STATUS(status, "failed to create connection \n");

   ////connection->user_data = &context;
   //connection->callback = connection_callback;
   //input_engine=engine1;
   //output_engine=engine2;

};

Connection::~Connection(){
	fprintf(stderr,"Destroying connection %s", connection->out->name);
	if (connection)
         mmal_connection_destroy(connection);
};
