TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CFLAGS_RELEASE   += -O3
QMAKE_LFLAGS_RELEASE   += -Wl,-O3

LIBS += -L/usr/local/lib -lnetcdf_c++4 -lnetcdf -ludunits2 -lpthread -lfreetype

INCLUDEPATH += /usr/include/freetype2

SOURCES += \
        main.cpp \
    grody/picohttpparser/picohttpparser.c \
    miniz/lupng.c \
    pngwrap.cpp \
    unitconverter.cpp \
    tile_render.cpp \
    legend_render.cpp \
    tools.cpp \
    types.cpp \
    style.cpp \
    json_helpers.cpp \
    datafile.cpp \
    clientdatacache.cpp \
    tile.cpp \
    clut.cpp \
    font.cpp \
    logger.cpp \
    grody/webserver.c \
    grody/thread.c \
    grody/io.c \
    grody/fork.c \
    grody/single_thread.c

HEADERS += \
    grody/picohttpparser/picohttpparser.h \
    grody/threaded_webserver.h \
    miniz/lupng.h \
    nlohmann/json.hpp \
    miniz/miniz.h \
    pngwrap.h \
    unitconverter.h \
    types.h \
    tools.h \
    style.h \
    dataquery.h \
    json_helpers.h \
    datafile.h \
    clientdatacache.h \
    tile.h \
    mercantile.h \
    clut.h \
    font.h \
    drawprimitives.hpp \
    logger.h \
    grody/webserver.h \
    grody/thread.h \
    grody/fork.h \
    grody/single_thread.h \
    miniz/miniz.hpp

DISTFILES += \
    read.me
