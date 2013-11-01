CC=g++
CFLAGS=-I. -lusb-1.0 -lusb
INCLUDES =

all: setup_daq receive_one select 

setup_daq:  setup_daq.c functions_usb.c usbfunctions.h Word.h Event.h Packet.h Level.h Signal.h
	${CC} -o setup_daq setup_daq.c    -I/usr/local/include -L. -lm -lc -L/usr/local/lib ${CFLAGS}

receive_one:  receive_one.c functions_usb.c usbfunctions.h Word.h Event.h Packet.h Level.h Signal.h
	${CC} -o receive_one receive_one.c    -I/usr/local/include -L. -lm -lc -L/usr/local/lib ${CFLAGS}

select:  select.c functions_usb.c usbfunctions.h Word.h Event.h Packet.h Level.h Signal.h
	${CC} -o select select.c    -I/usr/local/include -L. -lm -lc -L/usr/local/lib ${CFLAGS}

clean:
	rm  setup_daq receive_one select 

