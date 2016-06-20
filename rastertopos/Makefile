# Generated automatically from Makefile.in by configure.
# C compiler options
CC	= gcc
CFLAGS	= -g -O2
LIBS	= -lcups -lnsl 

# Source files
SRCS	= common.c rastertopos.c 

# Object files
OBJS	= $(SRCS:.c=.o)

# Make everything
all:	rastertopos 

# Make the rastertopos application
rastertopos: rastertopos.o
	$(CC) $(CFLAGS) -o rastertopos rastertopos.o -lcupsimage $(LIBS) -lm


# Dependencies
$(OBJS): config.h Makefile
clean:
	rm -rf *.o rastertopos
install:
	chmod +x setup.sh
	exec ./setup.sh
	
