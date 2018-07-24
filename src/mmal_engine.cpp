/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


//#ifndef mmal_engine
//#define mmal_engine

#include "mmal_engine.h"

//#endif




/** Callback from the input port.
 * Buffer has been consumed and is available to be used again. */
void mmal_engine::input_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   struct CONTEXT_T *ctx = (struct CONTEXT_T *)port->userdata;

   /* The encoder is done with the data, just recycle the buffer header into its pool */
   mmal_buffer_header_release(buffer);

   /* Kick the processing thread */
   //vcos_semaphore_post(&ctx->semaphore);
}

/** Callback from the output port.
 * Buffer has been produced by the port and is available for processing. */
void mmal_engine::output_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer )
{
   struct CONTEXT_T *ctx = (struct CONTEXT_T *)port->userdata;

   /* Queue the decoded video frame */
   mmal_queue_put(ctx->queue, buffer);

   /* Kick the processing thread */
   //vcos_semaphore_post(&ctx->semaphore);
}



uint8_t mmal_engine::set_video_input_port(uint16_t iwidth, uint16_t iheight, MMAL_FOURCC_T iformat){
	    
    /* Set format of video engine input port */
   MMAL_ES_FORMAT_T *format_in = engine->input[0]->format;
   format_in->type = MMAL_ES_TYPE_VIDEO;
   format_in->encoding = iformat;
   
   //width=VCOS_ALIGN_UP(iwidth,32);
   //height=VCOS_ALIGN_UP(iheight,16);
   
   width=iwidth;
   height=iheight;
   //format_in->es->video.width = VCOS_ALIGN_UP(iwidth, 32);
   //format_in->es->video.height = VCOS_ALIGN_UP(iheight,16);
   format_in->es->video.width = iwidth;
   format_in->es->video.height = iheight;
   
   
   format_in->es->video.frame_rate.num = 30;
   format_in->es->video.frame_rate.den = 1;
   format_in->es->video.par.num = 1;
   format_in->es->video.par.den = 1;
   format_in->es->video.crop.width = iwidth;
   format_in->es->video.crop.height = iheight;
   //format_in->es->video.crop.width = VCOS_ALIGN_UP(iwidth, 32);
   //format_in->es->video.crop.height = VCOS_ALIGN_UP(iheight,16);
   
   
   
   status = mmal_port_format_commit(engine->input[0]);
   CHECK_STATUS(status, "failed to commit input format");   

   
   
   
   
   
   
   return status;
	
	
};


uint8_t mmal_engine::set_video_output_port(uint16_t owidth, uint16_t oheight, MMAL_FOURCC_T oformat){
   MMAL_ES_FORMAT_T *format_out = engine->output[0]->format;
   format_out->type = MMAL_ES_TYPE_VIDEO;
   
   format_out->encoding = oformat;
   
   width=owidth;
   height=oheight;
  
   //format_out->es->video.width = VCOS_ALIGN_UP(owidth, 32);
   //format_out->es->video.height = VCOS_ALIGN_UP(oheight, 16);
   
   format_out->es->video.width = owidth;
   format_out->es->video.height = oheight;
   
   //Defaults
   format_out->es->video.frame_rate.num = 30;
   format_out->es->video.frame_rate.den = 1;
   format_out->es->video.par.num = 0; 
   format_out->es->video.par.den = 1;
   format_out->es->video.crop.width = owidth;
   format_out->es->video.crop.height = oheight;
   //format_out->es->video.crop.width = VCOS_ALIGN_UP(owidth, 32);
   //format_out->es->video.crop.height = VCOS_ALIGN_UP(oheight,16);

   status = mmal_port_format_commit(engine->output[0]);
   CHECK_STATUS(status, "failed to commit output format");   
   

   
   
   
   
   
   return status;
   
           	
};	


uint8_t mmal_engine::set_input_flag(uint32_t name) {
   status = mmal_port_parameter_set_boolean(engine->input[0], name, 1);
   CHECK_STATUS(status, "failed to set input port flag");	
   return status;  
};


uint8_t mmal_engine::set_output_flag(uint32_t name) {
   status = mmal_port_parameter_set_boolean(engine->output[0], name, 1);
      CHECK_STATUS(status, "failed to set input port flag");	
   return status; 
};


uint8_t mmal_engine::enable_video_input_port(){
	status = mmal_port_enable(engine->input[0], input_callback);
   CHECK_STATUS(status, "failed to enable input port");		
   
};	

uint8_t mmal_engine::enable_video_output_port(){
   status = mmal_port_enable(engine->output[0], output_callback);
   CHECK_STATUS(status, "failed to enable output port");  
   
};

uint8_t mmal_engine::create_input_pool(){
	/* The format of both ports is now set so we can get their buffer requirements and create
    * our buffer headers. We use the buffer pool API to create these. */
   engine->input[0]->buffer_num = engine->input[0]->buffer_num_recommended;
   engine->input[0]->buffer_size = engine->input[0]->buffer_size_recommended;
   
   pool_in = mmal_pool_create(engine->input[0]->buffer_num,
                              engine->input[0]->buffer_size);
   

   // Store a reference to our context in each port (will be used during callbacks) */
   engine->input[0]->userdata = (MMAL_PORT_USERDATA_T *)&context;
   
};	


uint8_t mmal_engine::create_output_pool(){
	engine->output[0]->buffer_num = engine->output[0]->buffer_num_recommended;
   engine->output[0]->buffer_size = engine->output[0]->buffer_size_recommended; 
	pool_out = mmal_pool_create(engine->output[0]->buffer_num,
                               engine->output[0]->buffer_size);
                               
   engine->output[0]->userdata = (MMAL_PORT_USERDATA_T *)&context; 
	
};	
	
uint8_t mmal_engine::enable() {
   
   //special cases 
     if (strcmp(name, "vc.ril.video_render") == 0) {
		//MMAL_PORT_T *preview_port = NULL;
        //preview_port = engine->input[0];

        //MMAL_DISPLAYREGION_T param;
        param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
        param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);

        param.set = MMAL_DISPLAY_SET_LAYER;
        param.layer = 1;

        param.set |= MMAL_DISPLAY_SET_ALPHA;
        param.alpha = 255;
      
        param.fullscreen = 1;
    
        mmal_port_parameter_set(engine->input[0], &param.hdr); 
      //  mmal_component_enable(engine);
	 } 	 
   
   
   status = mmal_component_enable(engine);
   CHECK_STATUS(status, "failed to enable component");
  
  
   
   /* Create a queue to store our decoded video frames. The callback we will get when
    * a frame has been decoded will put the frame into this queue. */
   context.queue = mmal_queue_create();
   
   buffsize=av_image_get_buffer_size(AV_PIX_FMT_YUV420P, VCOS_ALIGN_UP(width,32), VCOS_ALIGN_UP(height,16), 1);
    
   fprintf(stderr, "Constructing mmal engine %s ******************\n", name);
   
   
   if (input_port) {
   MMAL_ES_FORMAT_T *format_in = engine->input[0]->format;
   /* Display the port format */
   fprintf(stderr,"---------------------------------------------------\n");
   fprintf(stderr, "INPUT %s\n", engine->input[0]->name);
   fprintf(stderr, " type: %i, fourcc: %4.4s\n", format_in->type, (char *)&format_in->encoding);
   fprintf(stderr, " bitrate: %i, framed: %i\n", format_in->bitrate,
           !!(format_in->flags & MMAL_ES_FORMAT_FLAG_FRAMED));
   fprintf(stderr, " width: %i, height: %i, (%i,%i,%i,%i)\n",
           format_in->es->video.width, format_in->es->video.height,
           format_in->es->video.crop.x, format_in->es->video.crop.y,
           format_in->es->video.crop.width, format_in->es->video.crop.height);
           
   }        
   
   if (output_port) {
   MMAL_ES_FORMAT_T *format_out = engine->output[0]->format;
    /* Display the port format */
   fprintf(stderr,"---------------------------------------------------\n");
   fprintf(stderr, "OUTPUT %s\n", engine->output[0]->name);
   fprintf(stderr, " type: %i, fourcc: %4.4s\n", format_out->type, (char *)&format_out->encoding);
   fprintf(stderr, " bitrate: %i, framed: %i\n", format_out->bitrate,
           !!(format_out->flags & MMAL_ES_FORMAT_FLAG_FRAMED));
   fprintf(stderr, " width: %i, height: %i, (%i,%i,%i,%i)\n",
           format_out->es->video.width, format_out->es->video.height,
           format_out->es->video.crop.x, format_out->es->video.crop.y,
           format_out->es->video.crop.width, format_out->es->video.crop.height);
           
   }
   
   
   
   

   return status;
	
}		
	
uint8_t mmal_engine::run(AVFrame **frame, Buffer *outbuf)
{
	MMAL_BUFFER_HEADER_T *buffer;
	if ((buffer = mmal_queue_get(pool_in->queue)) != NULL)
      {
         
         mmal_buffer_header_mem_lock(buffer);
         av_image_copy_to_buffer(buffer->data, buffsize, (const uint8_t **)(*frame)->data, (*frame)->linesize,
                                AV_PIX_FMT_YUV420P, (*frame)->width, (*frame)->height, 1);
         buffer->length=buffsize;
         mmal_buffer_header_mem_unlock(buffer);
         
            

         fprintf(stderr, "%s sending >>>>> %i bytes\n", engine->input[0]->name, (int)buffer->length);
         status = mmal_port_send_buffer(engine->input[0], buffer);
         CHECK_STATUS(status, "failed to send buffer");
      }

     
      while ((buffer = mmal_queue_get(context.queue)) != NULL)
      {
         mmal_buffer_header_mem_lock(buffer);
         fprintf(stderr, "%s receiving %d bytes <<<<< frame\n", engine->output[0]->name, buffer->length);
         
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

     
      while ((buffer = mmal_queue_get(pool_out->queue)) != NULL)
      {
         status = mmal_port_send_buffer(engine->output[0], buffer);
         CHECK_STATUS(status, "failed to send buffer");
      }
     return status;    
}	


mmal_engine::mmal_engine(const char* iname):name(iname) {
	status = mmal_component_create(name, &engine);
    CHECK_STATUS(status, "failed to create engine");
    
    if (engine->input)
    input_port = engine->input[0];
    if (engine->output)
    output_port = engine->output[0]; 
          
};	
	
mmal_engine::~mmal_engine() {
	
   //if (engine->output[0]->name)	
     fprintf(stderr,"Destructing mmal engine %s\n", engine->name);
	//fprintf(stderr,"Destructing mmal engine\n");
	   /* Cleanup everything */
   if (engine)
      mmal_component_destroy(engine);
   if (pool_in)
      mmal_pool_destroy(pool_in);
   if (pool_out)
      mmal_pool_destroy(pool_out);
   if (context.queue)
      mmal_queue_destroy(context.queue);
      
      fprintf(stderr, "Done destructing engine");
};
