# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_XMeans
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

unix{
BOOST = /usr/local/boost_1_47_0
}else{
BOOST = E:/DEV/boost_1_47_0
}
INCLUDEPATH += $$BOOST

###########################
# Source Files            #
###########################
FORMS += paramsQTCluster.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			$$MLDEMOS/drawUtils.h \
			interfaceQTCluster.h \
			clustererQTClust.h \
			Clustering.h \
			vectorSpace.hpp

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			$$MLDEMOS/drawUtils.cpp \
			clustererQTClust.cpp \
			interfaceQTCLuster.cpp \
			Clustering.cpp \
			vectorSpace.cpp


###########################
# Dependencies            #
###########################
