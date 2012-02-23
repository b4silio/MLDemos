# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_Projections
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
DEFINES += OPENCV22

CONFIG(opencv22)|CONFIG(opencv21){
    CONFIG += opencv
}else{
    message("this plugin requires opencv, skipping")
}

###########################
# Source Files            #
###########################
FORMS += \
    paramsProjections.ui \
    paramsICA.ui \
    paramsLDA.ui \
    paramsKPCA.ui \
    paramsPCA.ui
HEADERS +=	\
        basicOpenCV.h \
        canvas.h \
        datasetManager.h \
        mymaths.h \
        eigen_pca.h \
        classifierLinear.h \
        classifierKPCA.h \
        interfaceProjections.h \
        pluginProjections.h \
        interfaceICAProjection.h \
        interfaceLDAProjection.h \
        interfacePCAProjection.h \
        interfaceKPCAProjection.h \
        projectorPCA.h \
        projectorKPCA.h \
        projectorICA.h \
        projectorLDA.h

SOURCES += 	\
    basicOpenCV.cpp \
    eigen_pca_kernel.cpp \
    classifierLinear.cpp \
    classifierKPCA.cpp \
    interfaceProjections.cpp \
    pluginProjections.cpp \
    interfaceICAProjection.cpp \
    interfaceLDAProjection.cpp \
    interfacePCAProjection.cpp \
    interfaceKPCAProjection.cpp \
    projectorPCA.cpp \
    projectorKPCA.cpp \
    projectorICA.cpp \
    projectorLDA.cpp

###########################
# Dependencies            #
###########################
HEADERS +=  JnS/Matutil.h \
            JnS/JnS.h

HEADERS +=  liblinear/linear.h \
            liblinear/tron.h \
            liblinear/blasp.h \
            liblinear/blas.h






























