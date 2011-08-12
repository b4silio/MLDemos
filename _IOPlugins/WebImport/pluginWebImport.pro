# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
QT += webkit
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
