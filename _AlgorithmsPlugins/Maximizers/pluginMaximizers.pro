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
FORMS += paramsMaximizers.ui paramsGA.ui paramsParticles.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			maximizeRandom.h \
			maximizePower.h \
			maximizeGA.h \
			maximizeParticles.h \
			maximizeGradient.h \
			pluginMaximizers.h \
			interfaceParticles.h \
			interfaceGA.h \
			gaPeon.h \
			gaTrainer.h \
    interfaceBasic.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			maximizeRandom.cpp \
			maximizePower.cpp \
			maximizeGA.cpp \
			maximizeParticles.cpp \
			maximizeGradient.cpp \
			pluginMaximizers.cpp \
			interfaceGA.cpp \
			interfaceParticles.cpp \
			gaPeon.cpp \
			gaTrainer.cpp \
    interfaceBasic.cpp

###########################
# Dependencies            #
###########################
