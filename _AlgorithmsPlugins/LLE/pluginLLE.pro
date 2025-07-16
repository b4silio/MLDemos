# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_LLE
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
FORMS += \
    paramsLLE.ui
SOURCES += pluginProjections.cpp \
    interfaceLLEProjection.cpp \
    projectorLLE.cpp

HEADERS += projectorLLE.h \
    interfaceLLEProjection.h

OTHER_FILES += \
    plugin.json
