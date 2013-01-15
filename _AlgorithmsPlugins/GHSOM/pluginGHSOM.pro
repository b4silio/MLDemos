# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_GHSOM
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

###########################
# Source Files            #
###########################
FORMS += \
    paramsGHSOM.ui
HEADERS +=	\
    glwidget.h \
    canvas.h \
    datasetManager.h \
    mymaths.h \
    interfaceGHSOM.h \
    projectorGHSOM.h

SOURCES += 	\
    interfaceGHSOM.cpp \
    projectorGHSOM.cpp

###########################
# Dependencies            #
###########################
HEADERS += \
    GHSOM/dataitem.h \
    GHSOM/dataloader.h \
    GHSOM/globals.h \
    GHSOM/neuron.h \
    GHSOM/neuronlayer.h \
    GHSOM/vector.h

SOURCES += \
    GHSOM/dataitem.cpp \
    GHSOM/dataloader.cpp \
    GHSOM/globals.cpp \
    GHSOM/neuron.cpp \
    GHSOM/neuronlayer.cpp \
    GHSOM/vector.cpp

