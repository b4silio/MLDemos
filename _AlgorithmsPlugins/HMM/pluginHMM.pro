# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_HMM
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
FORMS += paramsHMM.ui
HEADERS +=	\
	canvas.h \
	datasetManager.h \
	mymaths.h \
	drawUtils.h \
    dynamicalHMM.h \
    classifierHMM.h \
    interfaceHMMClassifier.h \
    interfaceHMMDynamic.h \
    pluginHMM.h \
    detectorHMM.h \
    interfaceHMMDetect.h

SOURCES += 	\
    pluginHMM.cpp \
    dynamicalHMM.cpp \
    classifierHMM.cpp \
    detectorHMM.cpp \
    interfaceHMMClassifier.cpp \
    interfaceHMMDetect.cpp \
    interfaceHMMDynamic.cpp

###########################
# Dependencies            #
###########################
HEADERS += \
	LAMP_HMM/discreteObsProb.h \
	LAMP_HMM/explicitDurationTrans.h \
	LAMP_HMM/gammaProb.h \
	LAMP_HMM/gaussianObsProb.h \
	LAMP_HMM/hmm.h \
	LAMP_HMM/initStateProb.h \
	LAMP_HMM/obs.h \
	LAMP_HMM/obsProb.h \
	LAMP_HMM/obsSeq.h \
	LAMP_HMM/plainStateTrans.h \
	LAMP_HMM/stateTrans.h \
	LAMP_HMM/utils.h \
	LAMP_HMM/vectorObsProb.h
