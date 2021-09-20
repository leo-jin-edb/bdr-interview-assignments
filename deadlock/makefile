FLAGS=-std=c++0x
LIBS=-lrt -lpthread

ifeq ($(MODE), debug)
FLAGS:= $(FLAGS) -g
endif

all:
	g++ $(FLAGS) -o deadlock *.cxx $(LIBS)

clean:
	rm -f *.o deadlock 
