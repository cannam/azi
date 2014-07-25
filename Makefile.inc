
SRC_DIR  := .

VAMPSDK_DIR  ?= ../vamp-plugin-sdk

PLUGIN_EXT	?= .so

CXX	?= g++
CC	?= gcc

CFLAGS := $(CFLAGS)
CXXFLAGS := $(CFLAGS) -I. -I$(VAMPSDK_DIR) $(CXXFLAGS)

LDFLAGS := $(LDFLAGS) 
PLUGIN_LDFLAGS := $(LDFLAGS) $(PLUGIN_LDFLAGS)

PLUGIN	:= azi$(PLUGIN_EXT)

PLUGIN_HEADERS := $(SRC_DIR)/Azi.h
PLUGIN_SOURCES := $(SRC_DIR)/Azi.cpp $(SRC_DIR)/plugins.cpp

HEADERS	     := $(PLUGIN_HEADERS)
SOURCES	     := $(PLUGIN_SOURCES)
OBJECTS	     := $(SOURCES:.cpp=.o)
OBJECTS	     := $(OBJECTS:.c=.o)

LIBS	:= $(VAMPSDK_DIR)/libvamp-sdk.a

all: $(PLUGIN)

$(PLUGIN):	$(OBJECTS) $(LIBS)
	$(CXX) -o $@ $^ $(LIBS) $(PLUGIN_LDFLAGS)

clean:		
	rm -f $(OBJECTS)

distclean:	clean
	rm -f $(PLUGIN)

depend:
	makedepend -Y -fMakefile.inc $(SOURCES) $(HEADERS)

# DO NOT DELETE

