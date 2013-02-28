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
FORMS += paramsGMM.ui paramsGMMcluster.ui paramsGMMregr.ui paramsGMMdynamic.ui \
    marginalWidget.ui
HEADERS +=	\
			canvas.h \
			datasetManager.h \
			mymaths.h \
			drawUtils.h \
			classifierGMM.h \
			clustererGMM.h \
			regressorGMR.h \
			dynamicalGMR.h \
			interfaceGMMClassifier.h \
			interfaceGMMCluster.h \
			interfaceGMMRegress.h \
			interfaceGMMDynamic.h \
			pluginGMM.h \
    marginalwidget.h

SOURCES += 	\
			classifierGMM.cpp \
			clustererGMM.cpp \
			regressorGMR.cpp \
			dynamicalGMR.cpp \
			interfaceGMMClassifier.cpp \
			interfaceGMMCluster.cpp \
			interfaceGMMRegress.cpp \
			interfaceGMMDynamic.cpp \
			pluginGMM.cpp \
    marginalwidget.cpp

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
