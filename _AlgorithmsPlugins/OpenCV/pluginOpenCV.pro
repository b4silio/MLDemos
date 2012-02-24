# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_OpenCV
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

CONFIG(opencv22)|CONFIG(opencv21){
	CONFIG += opencv
}else{
	message("this plugin requires opencv, skipping")
}

# opencv includes
win32:CONFIG(opencv22){
message("please adjust the include and lib paths to fit your system")
INCLUDEPATH += . "C:/DEV/OpenCV2.3-GCC/include/"
LIBS += -L"C:/DEV/OpenCV2.3-GCC/lib/"
#        "C:/OpenCV2.2/include/"
#LIBS += -L"C:/OpenCV2.2/lib/"
LIBS += -lopencv_core230 \
		-lopencv_features2d230 \
		-lopencv_highgui230 \
		-lopencv_imgproc230 \
		-lopencv_legacy230 \
		-lopencv_ml230
} else:CONFIG(opencv22) {
	INCLUDEPATH += /usr/local/include/
	LIBS += -L/usr/local/lib
	DEFINES += OPENCV22
	message("using opencv22")
	LIBS += \
		-lopencv_core \
		-lopencv_features2d \
		-lopencv_highgui \
		-lopencv_imgproc \
		-lopencv_legacy \
		-lopencv_ml
} else:CONFIG(opencv21) {
	INCLUDEPATH += /usr/local/include/
	LIBS += -L/usr/local/lib
	DEFINES += OPENCV21
	message("using opencv21")
	LIBS += \
		-lcv \
		-lcxcore \
		-lcvaux \
		-lml \
		-lhighgui
}

###########################
# Source Files            #
###########################
opencv{
FORMS += paramsBoost.ui paramsMLP.ui paramsMLPDynamic.ui paramsMLPRegress.ui
HEADERS +=	\
			canvas.h \
			datasetManager.h \
			mymaths.h \
			basicOpenCV.h \
			classifierBoost.h \
			classifierMLP.h \
			regressorMLP.h \
			dynamicalMLP.h \
			interfaceMLPClassifier.h \
			interfaceBoostClassifier.h \
			interfaceMLPRegress.h \
			interfaceMLPDynamic.h \
			pluginOpenCV.h

SOURCES += 	\
			basicOpenCV.cpp \
			classifierBoost.cpp \
			classifierMLP.cpp \
			regressorMLP.cpp \
			dynamicalMLP.cpp \
			interfaceMLPClassifier.cpp \
			interfaceBoostClassifier.cpp \
			interfaceMLPRegress.cpp \
			interfaceMLPDynamic.cpp \
			pluginOpenCV.cpp
}
