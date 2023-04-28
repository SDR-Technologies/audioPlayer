QT       -= core gui

TARGET = /opt/vmbase/extensions/audioPlugin
TEMPLATE = lib

DEFINES += VMPLUGINS_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS
LIBS += -lpthread -lpulse-simple -lsamplerate

SOURCES += \
    audioPlugin.cpp \
    audioplayer.cpp

HEADERS += \
    ConsumerProducer.h \
    vmplugins.h \
    vmsystem.h \
    plugin_factory.h \
    plugin_interface.h \
    vmtoolbox.h \
    vmtypes.h \
    audioplayer.h

DISTFILES += \
    README.md \
    example.js \ 
    CMakeLists.txt
