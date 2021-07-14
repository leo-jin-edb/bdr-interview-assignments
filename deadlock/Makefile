CC = g++
CFLAGS = --std=c++17

SRCS = graph.cc deadlock.cc

OBJS = $(SRCS:.cc=.o)
MAIN = deadlock

all:     $(MAIN)
	     @echo compiled....

$(MAIN): $(OBJS)
		 $(CC) $(CFLAGS) -o deadlock $(OBJS)

.cc.o:
		 $(CC) $(CFLAGS) -c $< -o $@
clean:
		 $(RM) *.o *~ $(MAIN)
