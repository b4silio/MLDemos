# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_mlpESMLR
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
LIBS += -L$$MLPATH/_3rdParty -l3rdParty

###########################
# Source Files            #
###########################
FORMS += paramsESMLR.ui
HEADERS +=	classifierESMLR.h \
			interfaceESMLRClassifier.h \
			pluginESMLR.h
SOURCES += 	classifierESMLR.cpp \
			interfaceESMLRClassifier.cpp \
			pluginESMLR.cpp

###########################
# Dependencies            #
###########################
