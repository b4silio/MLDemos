# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_Maximizers
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

###########################
# Source Files            #
###########################
FORMS += paramsMaximizers.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			maximizeRandom.h \
			maximizePower.h \
			interfaceMaximizers.h \
			pluginMaximizers.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			maximizeRandom.cpp \
			maximizePower.cpp \
			interfaceMaximizers.cpp \
			pluginMaximizers.cpp

###########################
# Dependencies            #
###########################
