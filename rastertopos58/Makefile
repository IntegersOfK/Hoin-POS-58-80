# Generated automatically from Makefile.in by configure.
# C compiler options
CC	= gcc
CFLAGS	= -g -O2
LIBS	= -lcups -lnsl 

# Source files
SRCS	= common.c rastertohoin58.c 

# Object files
OBJS	= $(SRCS:.c=.o)

# Make everything
all:	rastertopos58 

# Make the rastertos58 application
rastertopos58: rastertopos58.o
	$(CC) $(CFLAGS) -o rastertopos58 rastertopos58.o -lcupsimage $(LIBS) -lm


# Dependencies
$(OBJS): config.h Makefile
clean:
	rm -rf *.o rastertopos58
install:
	chmod +x setup.sh
	exec ./setup.sh
	
