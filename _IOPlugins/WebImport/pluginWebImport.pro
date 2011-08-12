# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = IO_WebImport
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)


###########################
# Source Files            #
###########################
FORMS += WebImport.ui

HEADERS +=	WebImport.h \
    parser.h

SOURCES += 	WebImport.cpp \
    parser.cpp
