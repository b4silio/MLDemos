# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = IO_PCAFaces
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
	-lopencv_ml \
#	-lopencv_calib3d \
#	-lopencv_contrib \
#	-lopencv_ffmpeg \
#	-lopencv_flann \
#	-lopencv_gpu \
#	-lopencv_objdetect \
#	-lopencv_ts \
#	-lopencv_video
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
