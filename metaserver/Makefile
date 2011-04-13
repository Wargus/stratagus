CXXFLAGS = -I../include -I/usr/include/SDL -DHAVE_STRCASESTR -W -Wall -O2 -Wno-write-strings
LDFLAGS = -lSDL -lsqlite3

metaserver: cmd.o db.o games.o netdriver.o main.o lowlevel.o
	$(CXX) $^ $(LDFLAGS) -o $@

lowlevel.o: ../network/lowlevel.cpp
	$(CXX) -c $^ $(CXXFLAGS) -o $@

clean:
	$(RM) *.o metaserver
