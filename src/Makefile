CC	?= gcc
CFLAGS	 = -DDEBUG -O2 -Wall
OBJS	 = main.o utils.o message.o osdepend.o usbdevfile.o usbops.o it9175.o it9175_usb.o tsthread.o
LIBS	 = -lpthread
TARGET	 = recfsusb2i

ifdef B25
LIBS	+= $(shell pkg-config --libs aribb25)
CFLAGS_utils	+= -DSTD_B25 $(shell pkg-config --cflags aribb25)
endif
ifeq ($(shell getconf LONG_BIT),32)
CFLAGS	+= -D_FILE_OFFSET_BITS=64
endif

all: $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

depend:
	$(CC) -MM $(OBJS:.o=.c) > Makefile.dep

-include Makefile.dep

utils.o :
	$(CC) $(CFLAGS) $(CFLAGS_utils) -c -o $@ $(@:.o=.c)

