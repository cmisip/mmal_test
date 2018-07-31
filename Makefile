IDIR =include 
ODIR =obj
SDIR =src

CC=g++

IFLAGS = -I/opt/vc/include -I/opt/vc/include/interface -I/opt/vc/include/containers
LFLAGS = -L/opt/vc/lib

LIBAVFLAGS = -lavutil -lavformat -lavcodec -lswscale
LIBMMALFLAGS = -lmmal -lbcm_host -lmmal_core -lmmal_components -lmmal_util -lvcos 

LDFLAGS =  $(LIBAVFLAGS) $(LIBMMALFLAGS)



CFLAGS=-I$(IDIR)  -std=gnu++11 $(IFLAGS) $(LFLAGS)




_DEPS = ffmpeg_camera.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))


_OBJ = main_template.o ffmpeg_camera.o mmal_engine.o buffer.o connection.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))



$(ODIR)/%.o: $(SDIR)/%.cpp 
	$(CC) -c -o $@ $< $(CFLAGS)
	



template: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f  $(ODIR)/*.o *~ template* include/*.gch
