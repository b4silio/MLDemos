# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
QT += webkit
NAME = IO_ImportTimeseries
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)


###########################
# Source Files            #
###########################
FORMS += \
    importTimeseries.ui

HEADERS += \
	parser.h \
	fileUtils.h \
	importTimeseries.h

SOURCES += \
	parser.cpp \
    importTimeseries.cpp
