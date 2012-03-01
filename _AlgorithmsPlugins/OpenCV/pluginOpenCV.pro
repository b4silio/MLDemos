# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_OpenCV
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
opencv{
FORMS += paramsBoost.ui paramsMLP.ui paramsMLPDynamic.ui paramsMLPRegress.ui
HEADERS +=	\
			canvas.h \
			datasetManager.h \
			mymaths.h \
			basicOpenCV.h \
			classifierBoost.h \
			classifierMLP.h \
			regressorMLP.h \
			dynamicalMLP.h \
			interfaceMLPClassifier.h \
			interfaceBoostClassifier.h \
			interfaceMLPRegress.h \
			interfaceMLPDynamic.h \
			pluginOpenCV.h

SOURCES += 	\
			basicOpenCV.cpp \
			classifierBoost.cpp \
			classifierMLP.cpp \
			regressorMLP.cpp \
			dynamicalMLP.cpp \
			interfaceMLPClassifier.cpp \
			interfaceBoostClassifier.cpp \
			interfaceMLPRegress.cpp \
			interfaceMLPDynamic.cpp \
			pluginOpenCV.cpp
}
