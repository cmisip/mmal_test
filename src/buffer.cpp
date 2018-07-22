#include "buffer.h"
#include <cstdlib>
                                   
Buffer::Buffer(uint32_t size){
	uint32_t *tmp=(uint32_t *)malloc(size);
	data=(uint8_t *)tmp;
	//data=(uint8_t*)malloc(size);
	length=size;
	
};

Buffer::~Buffer(){
	fprintf(stderr, "Destructing Buffer\n");
	free(data);
};	

                                   

