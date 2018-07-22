IDIR =include 
ODIR =obj
SDIR =src

CC=g++

IFLAGS = -I/opt/vc/include -I/opt/vc/include/interface
LFLAGS = -L/opt/vc/lib

LIBAVFLAGS = -lavutil -lavformat -lavcodec
LIBMMALFLAGS = -lmmal -lbcm_host -lmmal_core -lmmal_components -lmmal_util -lvcos 

LDFLAGS =  $(LIBAVFLAGS) $(LIBMMALFLAGS)



CFLAGS=-I$(IDIR)  -std=gnu++11 $(IFLAGS) $(LFLAGS)




_DEPS = ffmpeg_camera.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))


_OBJ = motion_vector.o ffmpeg_camera.o mmal_engine.o buffer.o connection.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))



$(ODIR)/%.o: $(SDIR)/%.cpp 
	$(CC) -c -o $@ $< $(CFLAGS)
	



motion: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f  $(ODIR)/*.o *~ motion_vector include/*.gch
