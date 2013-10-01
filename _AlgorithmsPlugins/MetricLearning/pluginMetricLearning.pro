# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin

NAME = mld_MetricLearning
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################

# the forms contain the widgets for changing the hyperparameters
FORMS += paramsCVO.ui

HEADERS += \
    canvas.h \
    datasetManager.h \
    mymaths.h \
    drawUtils.h \
    projectorCVO.h \
    interfaceCVOProjection.h \
    pluginMetricLearning.h \
    CVOLearner.h

SOURCES += \
    projectorCVO.cpp \
    interfaceCVOProjection.cpp \
    pluginMetricLearning.cpp \
    CVOLearner.cpp

OTHER_FILES += \
    plugin.json
