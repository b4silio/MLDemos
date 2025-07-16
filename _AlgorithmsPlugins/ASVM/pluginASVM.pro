# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_ASVM
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
FORMS += \
    paramsASVM.ui
HEADERS +=	\
    ASVMLearning.h \
    asvm.h \
    asvmdata.h \
    asvm_smo_solver.h \
    svm.h \
    util.h \
    canvas.h \
    datasetManager.h \
    mymaths.h \
    drawUtils.h \
    interfaceASVMDynamic.h \
    dynamicalASVM.h

SOURCES += 	\
    asvm.cpp \
    asvm_smo_solver.cpp \
    asvmdata.cpp \
    util.cpp \
    svm.cpp \
    interfaceASVMDynamic.cpp \
    dynamicalASVM.cpp

OTHER_FILES += \
    plugin.json
