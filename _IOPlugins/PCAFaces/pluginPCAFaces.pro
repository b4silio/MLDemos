# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = IO_PCAFaces
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

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
