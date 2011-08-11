# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = IO_PCAFaces
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
FORMS += PCAFaces.ui

HEADERS +=	PCAFaces.h \
			sampleManager.h \
			basicOpenCV.h \
			cameraGrabber.h \
			eigenFaces.h \
			projector.h \
			widget.h

SOURCES += 	PCAFaces.cpp \
			sampleManager.cpp \
			basicOpenCV.cpp \
			cameraGrabber.cpp \
			eigenFaces.cpp \
			projector.cpp \
			widget.cpp
