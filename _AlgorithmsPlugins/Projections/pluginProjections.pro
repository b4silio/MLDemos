# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_Projections
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
LIBS += -L$$MLPATH/_3rdParty -l3rdParty
###########################
# Source Files            #
###########################
FORMS += \
	paramsProjections.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			eigen_pca.h \
			classifierLinear.h \
			classifierKPCA.h \
    interfaceProjections.h \
    pluginProjections.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			eigen_pca_kernel.cpp \
			classifierLinear.cpp \
			classifierKPCA.cpp \
    interfaceProjections.cpp \
    pluginProjections.cpp

###########################
# Dependencies            #
###########################
HEADERS += 	JnS/Matutil.h \
			JnS/JnS.h

HEADERS += 	liblinear/linear.h \
			liblinear/tron.h \
			liblinear/blasp.h \
			liblinear/blas.h
