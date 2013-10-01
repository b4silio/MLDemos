# ##########################
# Configuration      #
# ##########################
QT += testlib

TARGET = UnitTesting
NAME = UnitTesting
MLPATH =..
DESTDIR = $$MLPATH

CONFIG += mainApp
include($$MLPATH/MLDemos_variables.pri)


# ##########################
# Source Files       #
# ##########################
HEADERS += \  
    testDatasetManager.h
SOURCES += \  
    testDatasetManager.cpp
