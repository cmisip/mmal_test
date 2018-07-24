//#ifndef connection
//#define connection

#include "connection.h"

//#endif
void Connection::input_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
   //CONTEXT_T *ctx = (struct CONTEXT_T *)port->userdata;
   mmal_buffer_header_release(buffer);
}

void Connection::output_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
   CONTEXT_T *ctx = (struct CONTEXT_T *)port->userdata;
   mmal_queue_put(ctx->queue, buffer);
}

uint8_t Connection::connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection)
{
   MMAL_STATUS_T status;

   status =  mmal_connection_create(connection, output_port, input_port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_OUTPUT);
   CHECK_STATUS(status, "failed to enable output port");
 
   return status;
}


uint8_t Connection::enable(){
	status =  mmal_connection_enable(connection);
	CHECK_STATUS(status, "failed to enable connection");
	if (input_port) {
	input_pool = mmal_pool_create(input_port->buffer_num,
                                  input_port->buffer_size);
    
    //Enable the input port and assign an input context
    input_port->userdata = (struct MMAL_PORT_USERDATA_T *)&contexti;
    
    status = mmal_port_enable(input_port, input_callback); 
    CHECK_STATUS(status, "failed to enable connection input port");
    
    
    }
    
    if (output_port) {
    output_pool = mmal_pool_create(output_port->buffer_num,
                                   output_port->buffer_size);

    //Create output context queue    
    
    contexto.queue = mmal_queue_create();
    
    // Store a reference to our context in each port (will be used during callbacks) 
    output_port->userdata = (struct MMAL_PORT_USERDATA_T *)&contexto; 
    
    //Enable the output port for resizerd and assign an output context
    status = mmal_port_enable(output_port, output_callback);
    CHECK_STATUS(status, "failed to enable connection output port");
    }
    
    
    
    
    
	
	
	buffsize=av_image_get_buffer_size(AV_PIX_FMT_YUV420P, input_engine->width, input_engine->height, 1);
	
	fprintf(stderr, "Constructing mmal connection %s ******************\n", connection->name);
	return status;
};


uint8_t Connection::run(AVFrame **frame, Buffer *outbuf){
	             MMAL_BUFFER_HEADER_T *buffer;
	            //send buffer with yuv420 data to pipeline input
	            
	            if (input_port) { 
                if ((buffer = mmal_queue_get(input_pool->queue)) != NULL)  {
                    
                    
                  mmal_buffer_header_mem_lock(buffer);
                  av_image_copy_to_buffer(buffer->data, buffsize, (const uint8_t **)(*frame)->data, (*frame)->linesize,
                                AV_PIX_FMT_YUV420P, (*frame)->width, (*frame)->height, 1);
                  buffer->length=buffsize;                                                                     //if we supply a time stamp to pts, the first buffer returned with the same time stamp is the matching data for the frame sent
                  mmal_buffer_header_mem_unlock(buffer);
                       
                  fprintf(stderr, "%s sending >>>>> %i bytes\n", input_port->name, (int)buffer->length);
                  status = mmal_port_send_buffer(input_port, buffer);
                  CHECK_STATUS(status, "failed to send buffer")     
                 }
			     }
			     
			     if (output_port) {
                  while ((buffer = mmal_queue_get(contexto.queue)) != NULL) {
					mmal_buffer_header_mem_lock(buffer);
                    fprintf(stderr, "%s receiving %d bytes <<<<< frame\n", output_port->name, buffer->length);
         
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
				  
				  while ((buffer = mmal_queue_get(output_pool->queue)) != NULL)
                  {
                    status = mmal_port_send_buffer(output_port, buffer);
                    CHECK_STATUS(status, "failed to send buffer");
                   }
                  } 
     return status;  
                 
	
};	

Connection::Connection(mmal_engine *engine1, mmal_engine *engine2):input_engine(engine1),output_engine(engine2){

    connect_ports(engine1->engine->output[0],engine2->engine->input[0], &connection);
    
    if (engine1->engine->input)
    input_port = engine1->engine->input[0];
    if (engine2->engine->output)
    output_port = engine2->engine->output[0];
    
};

Connection::~Connection(){
	fprintf(stderr,"Destroying connection %s\n", connection->out->name);
	if (connection)
         mmal_connection_destroy(connection);
    
    if (output_pool)
      mmal_pool_destroy(output_pool);
      
    if (input_pool)
      mmal_pool_destroy(input_pool);
     
    
    if (contexto.queue)
      mmal_queue_destroy(contexto.queue);
     
     fprintf(stderr,"Done Destroying connection \n");    
         
};
