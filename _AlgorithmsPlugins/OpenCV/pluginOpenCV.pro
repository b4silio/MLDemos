# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_OpenCV
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

# opencv includes
win32{
INCLUDEPATH += . \
         "C:/DEV/OpenCV2.2GCC/include/"
LIBS += -L"C:/DEV/OpenCV2.2GCC/lib/"
#        "C:/OpenCV2.2/include/"
#LIBS += -L"C:/OpenCV2.2/lib/"
LIBS += -lopencv_core229 \
        -lopencv_features2d229 \
        -lopencv_highgui229 \
        -lopencv_imgproc229 \
        -lopencv_legacy229 \
        -lopencv_ml229
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
FORMS += paramsBoost.ui paramsMLP.ui paramsKM.ui paramsMLPDynamic.ui paramsMLPRegress.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			basicOpenCV.h \
			classifierBoost.h \
			classifierMLP.h \
			regressorMLP.h \
			dynamicalMLP.h \
			clustererKM.h \
			kmeans.h \
			interfaceMLPClassifier.h \
			interfaceKMCluster.h \
			interfaceBoostClassifier.h \
			interfaceMLPRegress.h \
			interfaceMLPDynamic.h \
			pluginOpenCV.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			basicOpenCV.cpp \
			classifierBoost.cpp \
			classifierMLP.cpp \
			regressorMLP.cpp \
			dynamicalMLP.cpp \
			clustererKM.cpp \
			kmeans.cpp \
			interfaceMLPClassifier.cpp \
			interfaceKMCluster.cpp \
			interfaceBoostClassifier.cpp \
			interfaceMLPRegress.cpp \
			interfaceMLPDynamic.cpp \
			pluginOpenCV.cpp
