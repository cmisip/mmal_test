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
	
    
    status = mmal_port_enable(input_port, input_callback); 
    CHECK_STATUS(status, "failed to enable connection input port");
    
    
    }
    
    if (output_port) {
    
    //Enable the output port for resizerd and assign an output context
    status = mmal_port_enable(output_port, output_callback);
    CHECK_STATUS(status, "failed to enable connection output port");
    }
    
 
	
	buffsize=av_image_get_buffer_size(AV_PIX_FMT_YUV420P, input_engine->width, input_engine->height, 1);
	
	fprintf(stderr, "Constructing mmal connection %s ******************\n", connection->name);
	return status;
};


uint8_t Connection::create_output_pool(){
	
	   if (output_port) {
    output_pool = mmal_pool_create(output_port->buffer_num,
                                   output_port->buffer_size);

    //Create output context queue    
    
    contexto.queue = mmal_queue_create();
    
    // Store a reference to our context in each port (will be used during callbacks) 
    output_port->userdata = (struct MMAL_PORT_USERDATA_T *)&contexto; 
    
}
    
	
	
}	

uint8_t Connection::create_input_pool(){
	if (input_port) {
	input_pool = mmal_pool_create(input_port->buffer_num,
                                  input_port->buffer_size);
    
    //Enable the input port and assign an input context
    input_port->userdata = (struct MMAL_PORT_USERDATA_T *)&contexti;
    
    
    
    }
}	

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
					 fprintf(stderr, "OUTPUT----------------------------------------------");
					 
                  while ((buffer = mmal_queue_get(contexto.queue)) != NULL) {
					mmal_buffer_header_mem_lock(buffer);
                    fprintf(stderr, "%s receiving %d bytes <<<<< frame\n", output_port->name, buffer->length);
         
                    if (outbuf) {
                      memset(outbuf->data,0,outbuf->length);
                      memcpy(outbuf->data,buffer->data,buffer->length);
                      outbuf->length=buffer->length;
                      outbuf->flags=buffer->flags;
                      outbuf->cmd=buffer->cmd;
                      outbuf->pts=buffer->pts;
                      outbuf->dts=buffer->dts;
				    }
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
//Connect output port of engine1 to input port of engine 2
//However, input port here is the input of the connection (engine 1 input port ) and 
//the output port here is the output of the connection ( engine 2 output port )

//For this connection to work, engine1 must have an output port and engine 2 must have an input port
//Also, the buffers will be sent to engine1 input port and retrieved from engine 2 output port ( if it has one
//as some components dont have outputs such as the renderer).

//I think this will do a format copy between outconnect_port and inconnect_port and then commit and enable
//So both connect ports must be disabled first   
//It will also try to enable the input port of first connection and output port of second connection so if
//those are enabled, they need to be disabled first 
    if (input_engine->engine->input) {
      input_port = input_engine->engine->input[0];
      if (input_port->is_enabled)
        mmal_port_disable(input_port);
    }  
    if (output_engine->engine->output) {
      output_port = output_engine->engine->output[0];
      if (output_port->is_enabled)
        mmal_port_disable(output_port);
    }  
    if (input_engine->engine->output) {
      outconnect_port = input_engine->engine->output[0];
      if (outconnect_port->is_enabled)
        mmal_port_disable(outconnect_port);
    }  
    if (output_engine->engine->input) {
      inconnect_port = output_engine->engine->input[0]; 
      if (inconnect_port->is_enabled)
        mmal_port_disable(inconnect_port);
    }  

  
    connect_ports(outconnect_port,inconnect_port, &connection);
    
    //FIXME, probably should also need to destroy the input and output pool of the individual engines
    
    
};

Connection::Connection(Connection *connection1, Connection *connection2){
	input_engine=connection1->input_engine;
	output_engine=connection2->output_engine;
	if ( connection1->input_engine->input_port ) 
	if (connection1->input_engine->input_port->is_enabled) {
      input_port = connection1->input_engine->input_port;
      mmal_port_disable(input_port);
    } 
    
    if ( connection2->output_engine->output_port ) 
    if (connection2->output_engine->output_port->is_enabled) {
      output_port = connection2->output_engine->output_port;
      mmal_port_disable(output_port);
    }  
    if ( connection1->output_engine->output_port ) 
    if (connection1->output_engine->output_port->is_enabled) {
      outconnect_port = connection1->output_engine->output_port;
      mmal_port_disable(outconnect_port);
    }  
    if ( connection2->output_engine->input_port ) 
    if (connection2->output_engine->input_port->is_enabled) {
      inconnect_port = connection2->input_engine->input_port;  
      mmal_port_disable(inconnect_port);
    }  
    //These are valid connections already so the outconnect port and inconnect port are enabled.
    //Disable them here before connecting  
    
      
    connect_ports(outconnect_port,inconnect_port, &connection);
    
        //FIXME, probably should also need to destroy the input and output pool of the individual engines
        //FIXME, probably should also need to destory the input and output pool of the individual connections
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
