# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
# you can change the name to suit the name of your plugin
NAME = mld_ZeExample
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
# add here all the files in your project

# the forms contain the widgets for changing the hyperparameters
FORMS += paramsExample.ui

HEADERS +=	\
			canvas.h \
			datasetManager.h \
			mymaths.h \
			drawUtils.h \
            classifierExample.h \
            clustererExample.h \
            regressorExample.h \
            dynamicalExample.h \
            interfaceExampleClassifier.h \
            interfaceExampleCluster.h \
            interfaceExampleRegress.h \
            interfaceExampleDynamic.h \
            pluginExample.h

SOURCES += 	\
            classifierExample.cpp \
            clustererExample.cpp \
            regressorExample.cpp \
            dynamicalExample.cpp \
            interfaceExampleClassifier.cpp \
            interfaceExampleCluster.cpp \
            interfaceExampleRegress.cpp \
            interfaceExampleDynamic.cpp \
            pluginExample.cpp

###########################
# Dependencies            #
###########################
# add here the dependencies to third party libraries (e.g. the one in 3rdParty)
#HEADERS +=
