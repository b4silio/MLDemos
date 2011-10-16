# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = IO_CSVImport
MLPATH =../..

CONFIG(opencv22)|CONFIG(opencv21){
    CONFIG += opencv
}else{
    message("this plugin requires opencv, skipping")
}

include($$MLPATH/MLDemos_variables.pri)

###########################
# Source Files            #
###########################
FORMS += CSVImport.ui

HEADERS += CSVImport.h \
            basicOpenCV.h \
            parser.h \
            widget.h \
    pcaprojection.h

SOURCES += CSVImport.cpp \
            basicOpenCV.cpp \
            parser.cpp \
            widget.cpp \
    pcaprojection.cpp


