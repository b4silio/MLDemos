# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_GMM
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

###########################
# Source Files            #
###########################
FORMS += paramsGMM.ui paramsGMMcluster.ui paramsGMMregr.ui paramsGMMdynamic.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			$$MLDEMOS/drawUtils.h \
			classifierGMM.h \
			clustererGMM.h \
			regressorGMR.h \
			dynamicalGMR.h \
			interfaceGMMClassifier.h \
			interfaceGMMCluster.h \
			interfaceGMMRegress.h \
			interfaceGMMDynamic.h \
			pluginGMM.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			$$MLDEMOS/drawUtils.cpp \
			classifierGMM.cpp \
			clustererGMM.cpp \
			regressorGMR.cpp \
			dynamicalGMR.cpp \
			interfaceGMMClassifier.cpp \
			interfaceGMMCluster.cpp \
			interfaceGMMRegress.cpp \
			interfaceGMMDynamic.cpp \
			pluginGMM.cpp

###########################
# Dependencies            #
###########################
HEADERS +=	\
	fgmm/em.h \
	fgmm/fgmm++.hpp \
	fgmm/fgmm.h \
	fgmm/gaussian.h \
	fgmm/regression.h \
	fgmm/smat.h

SOURCES += \
	fgmm/em.cpp \
	fgmm/gaussian.cpp \
	fgmm/gmm.cpp \
	fgmm/gmmregression.cpp \
	fgmm/smat.cpp \
	fgmm/update.cpp

