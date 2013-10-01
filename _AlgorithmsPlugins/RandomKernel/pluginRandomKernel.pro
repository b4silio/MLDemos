# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_RandomKernel
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
FORMS += \
    paramsRSVM.ui \
    paramsRGPR.ui
SOURCES += pluginRandomKernel.cpp \
    classifierRSVM.cpp \
    interfaceRSVMClassifier.cpp \
    randomKernelUtils.cpp \
    interfaceRGPRegressor.cpp \
    regressorRGPR.cpp
#    liblinear_debug/predict.c \
#    liblinear_debug/train.c \

HEADERS += pluginRandomKernel.h \
    classifierRSVM.h \
    interfaceRSVMClassifier.h \
    randomKernelUtils.h \
    interfaceRGPRegressor.h \
    regressorRGPR.h

OTHER_FILES += \
    plugin.json
