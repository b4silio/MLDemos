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
INCLUDEPATH += . "C:/DEV/OpenCV2.2GCC/include/"
LIBS += -L"C:/DEV/OpenCV2.2GCC/lib/"
#        "C:/OpenCV2.2/include/"
#LIBS += -L"C:/OpenCV2.2/lib/"
LIBS += -lopencv_core229 \
		-lopencv_features2d229 \
		-lopencv_highgui229 \
		-lopencv_imgproc229 \
		-lopencv_legacy229 \
		-lopencv_ml229 \
		-lopencv_video229 \
		-lopencv_flann229 \
		-lopencv_calib3d229
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
	#	-lopencv_calib3d \
	#	-lopencv_contrib \
	#	-lopencv_ffmpeg \
	#	-lopencv_flann \
	#	-lopencv_gpu \
	#	-lopencv_objdetect \
	#	-lopencv_ts \
	#	-lopencv_video
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
