# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_LWPR
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
LIBS += -L$$MLPATH/_3rdParty -l3rdParty
###########################
# Source Files            #
###########################
FORMS += paramsLWPRDynamic.ui paramsLWPRRegress.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			regressorLWPR.h \
			dynamicalLWPR.h \
			interfaceLWPRRegress.h \
			interfaceLWPRDynamic.h \
			pluginLWPR.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			regressorLWPR.cpp \
			dynamicalLWPR.cpp \
			interfaceLWPRRegress.cpp \
			interfaceLWPRDynamic.cpp \
			pluginLWPR.cpp

###########################
# Dependencies            #
###########################
HEADERS += \
	lwpr/lwpr.h \
	lwpr/lwpr.hh \
	lwpr/lwpr_aux.h \
	lwpr/lwpr_binio.h \
	lwpr/lwpr_config.h \
	lwpr/lwpr_config_def.h \
	lwpr/lwpr_math.h \
	lwpr/lwpr_mem.h \
	lwpr/lwpr_xml.h

