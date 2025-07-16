# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_MeanShift
MLPATH = ../..

include($$MLPATH/MLDemos_variables.pri)

###########################
# Source Files            #
###########################
# add here all the files in your project

# the forms contain the widgets for changing the hyperparameters
FORMS += \
    paramsMeanShift.ui

HEADERS +=  \
    canvas.h \
    datasetManager.h \
    mymaths.h \
    drawUtils.h \
    clustererMeanShift.h \
    interfaceMeanShiftCluster.h \
    pluginMeanShift.h \
    MeanShift/MeanShift.h

SOURCES += \
    clustererMeanShift.cpp \
    interfaceMeanShiftCluster.cpp \
    pluginMeanShift.cpp \
    MeanShift/MeanShift.cpp

OTHER_FILES += \
    plugin.json
