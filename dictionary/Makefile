CC=gcc
CFLAGS=-I$(IDIR)
LIBS=
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CFLAGS += -D LINUX
	LIBS += -lrt -pthread
endif
LIBS += -lm
ifeq ($(UNAME_S),Darwin)
	CC=/usr/bin/clang
	CFLAGS += -D OSX
endif
UNAME_P := $(shell uname -p)
ifeq ($(UNAME_P),x86_64)
	CFLAGS += -D AMD64
endif
ifneq ($(filter %86,$(UNAME_P)),)
	CFLAGS += -D IA32
endif
ifneq ($(filter arm%,$(UNAME_P)),)
	CFLAGS += -D ARM
endif
IDIR =../include
ODIR=obj
LDIR =../lib

_DEPS =
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = dict.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

dict: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
