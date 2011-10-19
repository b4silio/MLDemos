# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = IO_CSVImport
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

###########################
# Source Files            #
###########################
FORMS += CSVImport.ui

HEADERS += CSVImport.h \
            parser.h

SOURCES += CSVImport.cpp \
            parser.cpp


