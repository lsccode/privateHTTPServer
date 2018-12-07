QUIET := @

#CROSS     = arm-linux-gnueabihf-
CXX      := $(CROSS)g++
CC       := $(CROSS)gcc
CPPFLAGS += -Wall # -Wno-note
LDFLAGS  += -lpthread
ARFLAGS  := rf

COMPILE_C       = $(QUIET)echo " GCC    $@    "; $(COMPILE.c)
COMPILE_CPP     = $(QUIET)echo " G++    $@    "; $(COMPILE.cpp)
LINK            = $(QUIET)echo " G++ -o $@ $^ "; $(LINK.cc)
ARLIB           = $(QUIET)echo " AR  -o $@ $^ "; $(AR) $(ARFLAGS)


SRC_C_FILES     := $(wildcard *.c)
SRC_C_OBJS      := $(SRC_C_FILES:.c=.o)
CURRENT_TARGET  := $(SRC_C_OBJS:.o=)

SRC_CPP_FILES   := $(wildcard *.cpp)
SRC_CPP_OBJS    := $(SRC_CPP_FILES:.cpp=.o)
CURRENT_TARGET  += $(basename $(SRC_CPP_OBJS))

INSTALL_PATH_DEST := /etc/thttpd/html

ALL += $(CURRENT_TARGET) 

all:$(CURRENT_TARGET)

install:
	@install -m 777 -s --strip-program=strip $(CURRENT_TARGET) $(INSTALL_PATH_DEST)
clean:
	@echo "clean *.o $(SRC_C_OBJS) $(SRC_CPP_OBJS) $(CURRENT_TARGET) *.d ";$(RM) $(SRC_C_OBJS) $(SRC_CPP_OBJS) $(CURRENT_TARGET) *.d *.d.*

.c.o:
	$(COMPILE_C) -o $@ $< 1>&2
	
.cpp.o:
	$(COMPILE_CPP) -o $@ $<
    
%.d: %.cpp
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
                
sinclude $(SRC_CPP_FILES:.cpp=.d)
sinclude $(SRC_C_FILES:.c=.d)
	
print:
	@echo "COMPILE.c   = $(COMPILE.c)" 
	@echo "COMPILE.cpp = $(COMPILE.cpp)" 
	@echo "LINK.cc     = $(LINK.cc)"
	@echo "CURRENT_TARGET = $(CURRENT_TARGET)"
