COMP := g++ -g 
DEBUG := 
LDFLAGS :=  
OPTI := 
INCL := 

LIBS :=  -lnsl -lrt -lpthread 

OBJS :=   MemoryManager.o Config.o WordManager.o Os.o Mutex.o 

 

dict : $(OBJS) dict.o MemoryMaster.o 
	$(COMP) -o dict  $(OBJS) dict.o  $(LIBS)
	$(COMP) -o MemoryMaster  $(OBJS) MemoryMaster.o  $(LIBS)


%.o:  %.cpp
	$(COMP) $(DEBUG) -c  $(OPTI) $(INCL) $< -o $(notdir $@)

.PHONY: clean
clean:
	rm -f *.o dict MemoryMaster
install:
