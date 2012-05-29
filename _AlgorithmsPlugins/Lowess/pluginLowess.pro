# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
# you can change the name to suit the name of your plugin
NAME = mld_Lowess
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
# add here all the files in your project

# the forms contain the widgets for changing the hyperparameters
FORMS += paramsLowess.ui

HEADERS +=	\
			canvas.h \
			datasetManager.h \
			mymaths.h \
			drawUtils.h \
            regressorLowess.h \
            interfaceLowess.h \
            pluginLowess.h \
    lowessHelpers.h

SOURCES += 	\
            regressorLowess.cpp \
            interfaceLowess.cpp \
            pluginLowess.cpp

###########################
# Dependencies            #
###########################
# add here the dependencies to third party libraries (e.g. the one in 3rdParty)
HEADERS += \
		    ../../_3rdParty/GSL_fit/gsl_fit.h
