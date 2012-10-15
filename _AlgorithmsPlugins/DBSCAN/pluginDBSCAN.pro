# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
# you can change the name to suit the name of your plugin
NAME = mld_DBSCAN
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

DEFINES += BOOST_TT_HAS_OPERATOR_HPP_INCLUDED
###########################
# Source Files            #
###########################
# add here all the files in your project

# the forms contain the widgets for changing the hyperparameters
FORMS += paramsDBSCAN.ui \
    graphDBSCAN.ui

HEADERS +=	\
			canvas.h \
			datasetManager.h \
			mymaths.h \
			drawUtils.h \
    interfaceDBSCAN.h \
    distance.h \
    clustererDBSCAN.h \
    pluginDBSCAN.h

SOURCES += 	\
    interfaceDBSCAN.cpp \
    clustererDBSCAN.cpp \
    pluginDBSCAN.cpp

###########################
# Dependencies            #
###########################
# add here the dependencies to third party libraries (e.g. the one in 3rdParty)
#HEADERS +=
