# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_LinearMethods
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

# opencv includes
win32{
INCLUDEPATH += . \
        "C:/OpenCV2.2/include/"
LIBS += -L"C:/OpenCV2.2/lib/"
LIBS += -lopencv_core220 \
        -lopencv_features2d220 \
        -lopencv_highgui220 \
        -lopencv_imgproc220 \
        -lopencv_legacy220 \
        -lopencv_ml220
}else{
INCLUDEPATH += /usr/local/include/
LIBS += -L/usr/local/lib
LIBS += \
	-lopencv_core \
	-lopencv_features2d \
	-lopencv_highgui \
	-lopencv_imgproc \
	-lopencv_legacy \
        -lopencv_ml
}

###########################
# Source Files            #
###########################
FORMS += paramsLinear.ui paramsBoost.ui paramsMLP.ui paramsKM.ui paramsMLPDynamic.ui paramsMLPRegress.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			basicOpenCV.h \
			classifierBoost.h \
			classifierLinear.h \
			classifierMLP.h \
			regressorMLP.h \
			dynamicalMLP.h \
			clustererKM.h \
			kmeans.h \
			interfaceMLPClassifier.h \
			interfaceLinearClassifier.h \
			interfaceKMCluster.h \
			interfaceBoostClassifier.h \
			interfaceMLPRegress.h \
			interfaceMLPDynamic.h \
			pluginLinear.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			basicOpenCV.cpp \
			classifierBoost.cpp \
			classifierLinear.cpp \
			classifierMLP.cpp \
			regressorMLP.cpp \
			dynamicalMLP.cpp \
			clustererKM.cpp \
			kmeans.cpp \
			interfaceMLPClassifier.cpp \
			interfaceLinearClassifier.cpp \
			interfaceKMCluster.cpp \
			interfaceBoostClassifier.cpp \
			interfaceMLPRegress.cpp \
			interfaceMLPDynamic.cpp \
			pluginLinear.cpp

###########################
# Dependencies            #
###########################
HEADERS += 	JnS/Matutil.h \
			JnS/JnS.h
SOURCES += 	JnS/Matutil.cpp \
			JnS/JnS.cpp
