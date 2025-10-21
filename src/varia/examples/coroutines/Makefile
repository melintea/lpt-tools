#
#
#

TARGET   = corodebug

CXXFLAGS = -I../../../../include -ggdb -std=c++23 -O0 -fcoroutines -fdump-lang-coro # g++ > 15.1
LDFLAGS  = -Wl,-Bstatic -lstdc++exp -Wl,-Bdynamic
CXX      = /opt/gcc152/bin/g++

all: $(TARGET)

$(TARGET): debug.o  
	$(CXX) $< $(LDFLAGS) -o $(TARGET)
	
%.o: %.cpp Makefile ../../../../include/lpt/coroutines/*.h*
	$(CXX) $(CXXFLAGS) -c $<

run: $(TARGET)
	LD_LIBRARY_PATH=/opt/gcc152/lib64 ./$(TARGET)

debug: $(TARGET)
	LD_LIBRARY_PATH=/opt/gcc152/lib64 cgdb -d /opt/gdb163/bin/gdb ./$(TARGET)

clean:
	-rm *.o *.coro $(TARGET) 
