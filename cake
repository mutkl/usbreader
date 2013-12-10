make[1]: Entering directory `/home/vgenty/usbreader/src'
g++ -I. -I/usr/local/include -I/usr/include/root -c setup_daq.cxx
g++ -I. -I/usr/local/include -I/usr/include/root -c Word.cxx
g++ -I. -I/usr/local/include -I/usr/include/root -c Event.cxx
g++ -I. -I/usr/local/include -I/usr/include/root -c Packet.cxx
g++ -I. -I/usr/local/include -I/usr/include/root -c Signal.cxx
g++ -I. -I/usr/local/include -I/usr/include/root -c Level.cxx
g++ -m32 -L. -L/usr/local/lib -L/usr/lib/i386-linux-gnu -lusb-1.0 -lusb -lm -lc -lGui -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic  -o setup_daq setup_daq.o Word.o Event.o Packet.o Signal.o Level.o
make[1]: Leaving directory `/home/vgenty/usbreader/src'
