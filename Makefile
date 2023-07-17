
####### Compiler, tools and options

CC			= gcc
CXX			= g++
CFLAGS		= -pipe -O3 -Wall -W -fPIC 
CXXFLAGS	= $(CFLAGS) -std=gnu++1z
INCPATH		= -I. -I/usr/include/freetype2 -I/usr/include

DEL_FILE	= rm -f

LINK		= g++
LFLAGS		= 
#LFLAGS		= -Wl,-O3
LIBS		= -L/usr/local/lib -lnetcdf_c++4 -lnetcdf -ludunits2 -lpthread -lfreetype

####### Files
CPP_H = clientdatacache \
		datafile \
		font \
		json_helpers \
		logger \
		pngwrap \
		style \
		tile \
		tools \
		types \
		unitconverter \
		clut \
		\
		miniz/lupng \
		\
		grody/webserver \
		grody/thread \
		grody/single_thread \
		grody/fork \


SOURCES	= $(CPP_H:=.cpp)\
		main.cpp \
		legend_render.cpp \
		tile_render.cpp \
		\
		grody/picohttpparser/picohttpparser.c \
		\
		grody/io.c \

HEADERS	= $(CPP_H:=.h)\
		dataquery.h \
		drawprimitives.hpp \
		mercantile.h \
		\
		nlohmann/json.hpp \
		\
		grody/picohttpparser/picohttpparser.h \
		\
		grody/threaded_webserver.h \


OBJcpp	= $(SOURCES:.cpp=.o)
OBJECTS = $(OBJcpp:.c=.o)

TARGET	= application

####### Build rules
all: $(TARGET)

debug:	CFLAGS		= -pipe -DDEBUG -g -Wall -W -fPIC 
debug:	CXXFLAGS	= $(CFLAGS) -std=gnu++1z
debug:	all

clean:  
	-$(DEL_FILE) $(OBJECTS)

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $@ $^ $(LIBS)

####### Compile
$(HEADERS):

%.o: %.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<
%.o: grody/picohttpparser/%.c grody/%.c miniz/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<
