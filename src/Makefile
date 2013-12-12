CXX    := clang++
GXX    := g++
CFLAGS := -O3 -std=c++11 -I. -I/usr/local/include -I/usr/include/root
LFLAGS := -m32 -L. -L/usr/local/lib -L/usr/lib/i386-linux-gnu -lusb-1.0 -lusb -lm -lc -lGui -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic 
PFLAGS := -lpqxx -lpq

all: setup_daq receive_all receive_one analyze asyn

setup_daq: setup_daq.o Word.o Event.o Packet.o Signal.o Level.o
	$(GXX) -o $@ $^ $(LFLAGS)

receive_all: receive_all.o Word.o Event.o Packet.o Level.o Signal.o
	$(GXX) -o $@ $^ $(LFLAGS)

receive_one: receive_one.o Word.o Event.o Packet.o Level.o Signal.o
	$(GXX) -o $@ $^ $(LFLAGS) $(PFLAGS)

analyze: analyze.o Signal.o Level.o Packet.o Event.o Word.o
	$(GXX) -o $@ $^ $(LFLAGS) $(PFLAGS)

asyn: asyn.o Signal.o Level.o Packet.o Event.o Word.o
	$(GXX) -o $@ $^ $(LFLAGS) $(PFLAGS)

analyze.o: analyze.cxx Signal.h Level.h Packet.h Event.h Word.h Signal.cxx Level.cxx Packet.cxx Event.cxx Word.cxx
	$(GXX) $(CFLAGS) -c analyze.cxx	

receive_all.o: receive_all.cxx Word.h Event.h Level.h Packet.h Signal.h Signal.cxx Packet.cxx Event.cxx Level.cxx Word.cxx
	$(GXX) $(CFLAGS) -c receive_all.cxx

receive_one.o: receive_one.cxx Word.h Event.h Level.h Packet.h Signal.h Signal.cxx Packet.cxx Event.cxx Level.cxx Word.cxx
	$(GXX) $(CFLAGS) -c receive_one.cxx

setup_daq.o: setup_daq.cxx usbfunctions.h functions_usb.h Word.h Event.h Level.h Packet.h Signal.h Signal.cxx Packet.cxx Event.cxx Level.cxx Word.cxx
	$(GXX) $(CFLAGS) -c setup_daq.cxx

asyn.o: asyn.cxx usbfunctions.h functions_usb.h Word.h Event.h Level.h Packet.h Signal.h Signal.cxx Packet.cxx Event.cxx Level.cxx Word.cxx
	$(GXX) $(CFLAGS) -c asyn.cxx

Word.o: Word.h Word.cxx
	$(GXX) $(CFLAGS) -c Word.cxx

Event.o: Event.h Event.cxx Word.h Word.cxx
	$(GXX) $(CFLAGS) -c Event.cxx

Level.o: Level.h Level.cxx Event.h Packet.h Signal.h Event.cxx Packet.cxx Signal.cxx
	$(GXX) $(CFLAGS) -c Level.cxx

Signal.o: Signal.h Signal.cxx
	$(GXX) $(CFLAGS) -c Signal.cxx

Packet.o: Packet.h Packet.cxx Word.h Word.cxx Event.h Event.cxx
	$(GXX) $(CFLAGS) -c Packet.cxx
clean:
	rm -rf *.o setup_daq receive_all receive_one
