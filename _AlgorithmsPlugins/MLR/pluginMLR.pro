# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_MLR
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
LIBS += -L$$MLPATH/_3rdParty -l3rdParty

###########################
# Source Files            #
###########################
FORMS += paramsESMLR.ui
HEADERS +=	$$MLDEMOS/canvas.h \
			$$MLDEMOS/mymaths.h \
			MixtureLogisticRegression.h \
			EvolutionStrategy.h \
			classifierESMLR.h \
			interfaceESMLRClassifier.h \
			pluginMLR.h
SOURCES += 	$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/mymaths.cpp \
			MixtureLogisticRegression.cpp \
			EvolutionStrategy.cpp \
			classifierESMLR.cpp \
			interfaceESMLRClassifier.cpp \
			pluginMLR.cpp

###########################
# Dependencies            #
###########################
