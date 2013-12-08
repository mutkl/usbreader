CXX    := clang++
CFLAGS := -I. -I/usr/local/include -I/usr/include/root
LFLAGS := -m32 -L. -L/usr/local/lib -L/usr/lib/i386-linux-gnu -lusb-1.0 -lusb -lm -lc -lGui -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic

all: setup_daq receive_all

setup_daq: setup_daq.o Word.o Event.o Packet.o Signal.o Level.o
	$(CXX) $(LFLAGS) -o $@ $^

receive_all: receive_all.o Word.o Event.o Packet.o Level.o Signal.o
	$(CXX) $(LFLAGS) -o $@ $^

receive_all.o: receive_all.cxx Word.h Event.h Level.h Packet.h Signal.h Signal.cxx Packet.cxx Event.cxx Level.cxx Word.cxx
	$(CXX) $(CFLAGS) -c receive_all.cxx

setup_daq.o: setup_daq.cxx usbfunctions.h functions_usb.h Word.h Event.h Level.h Packet.h Signal.h Signal.cxx Packet.cxx Event.cxx Level.cxx Word.cxx
	$(CXX) $(CFLAGS) -c setup_daq.cxx

Word.o: Word.h Word.cxx
	$(CXX) $(CFLAGS) -c Word.cxx

Event.o: Event.h Event.cxx Word.h Word.cxx
	$(CXX) $(CFLAGS) -c Event.cxx

Level.o: Level.h Level.cxx Event.h Packet.h Signal.h Event.cxx Packet.cxx Signal.cxx
	$(CXX) $(CFLAGS) -c Level.cxx

Signal.o: Signal.h Signal.cxx
	$(CXX) $(CFLAGS) -c Signal.cxx

Packet.o: Packet.h Packet.cxx Word.h Word.cxx Event.h Event.cxx
	$(CXX) $(CFLAGS) -c Packet.cxx
clean:
	rm -rf *.o
