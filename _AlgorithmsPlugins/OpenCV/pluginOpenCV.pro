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
#opencv{
FORMS += paramsBoost.ui paramsMLP.ui paramsMLPDynamic.ui paramsMLPRegress.ui paramsGBRegress.ui
HEADERS +=	\
			canvas.h \
			datasetManager.h \
			mymaths.h \
			basicOpenCV.h \
			classifierBoost.h \
			classifierMLP.h \
			regressorMLP.h \
            dynamicalMLP.h \
            regressorGB.h \
            interfaceMLPClassifier.h \
			interfaceBoostClassifier.h \
			interfaceMLPRegress.h \
            interfaceMLPDynamic.h \
            interfaceGBRegress.h \
            pluginOpenCV.h

SOURCES += 	\
			basicOpenCV.cpp \
			classifierBoost.cpp \
			classifierMLP.cpp \
            regressorMLP.cpp \
			dynamicalMLP.cpp \
            regressorGB.cpp \
            interfaceMLPClassifier.cpp \
			interfaceBoostClassifier.cpp \
			interfaceMLPRegress.cpp \
			interfaceMLPDynamic.cpp \
            interfaceGBRegress.cpp \
            pluginOpenCV.cpp
#}
