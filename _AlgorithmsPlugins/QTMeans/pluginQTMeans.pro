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
			canvas.h \
			datasetManager.h \
			mymaths.h \
			drawUtils.h \
			interfaceQTCluster.h \
			clustererQTClust.h \
			Clustering.h \
			vectorSpace.hpp

SOURCES += 	\
			clustererQTClust.cpp \
			interfaceQTCLuster.cpp \
			Clustering.cpp \
			vectorSpace.cpp
