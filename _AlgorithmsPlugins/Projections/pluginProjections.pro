# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_Projections
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
FORMS += \
	paramsProjections.ui
HEADERS +=	\
			canvas.h \
			datasetManager.h \
			mymaths.h \
			eigen_pca.h \
			classifierLinear.h \
			classifierKPCA.h \
			interfaceProjections.h \
			pluginProjections.h

SOURCES += 	\
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
