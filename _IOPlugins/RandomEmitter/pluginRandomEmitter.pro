# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = IO_RandomEmitter
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

###########################
# Source Files            #
###########################
HEADERS +=	interfaceRandomEmitter.h

SOURCES += 	interfaceRandomEmitter.cpp
