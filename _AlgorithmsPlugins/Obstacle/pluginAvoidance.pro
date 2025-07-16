# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_DSAvoid
MLPATH =../..

include("$$MLPATH/MLDemos_variables.pri")
###########################
# Source Files            #
###########################
HEADERS +=	\
    canvas.h \
    datasetManager.h \
    mymaths.h \
    interfaces.h \
    obstacles.h \
    DSAvoid.h \
    interfaceAvoidance.h

SOURCES += 	\
    DSAvoid.cpp \
    interfaceAvoidance.cpp

OTHER_FILES += \
    plugin.json
