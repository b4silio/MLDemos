# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_Flame
MLPATH = ../..

include($$MLPATH/MLDemos_variables.pri)

###########################
# Source Files            #
###########################
# add here all the files in your project

# the forms contain the widgets for changing the hyperparameters
FORMS += \
    paramsFlame.ui

HEADERS +=  \
    canvas.h \
    datasetManager.h \
    mymaths.h \
    drawUtils.h \
    clustererFlame.h \
    flame.h \
    interfaceFlameCluster.h \
    pluginFlame.h

SOURCES += \
    clustererFlame.cpp \
    interfaceFlameCluster.cpp \
    pluginFlame.cpp \
    flame.cpp

OTHER_FILES += \
    plugin.json
