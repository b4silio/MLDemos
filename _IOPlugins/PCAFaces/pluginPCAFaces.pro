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
			widget.h \
    pcaprojector.h

SOURCES += 	PCAFaces.cpp \
			sampleManager.cpp \
			basicOpenCV.cpp \
			cameraGrabber.cpp \
			eigenFaces.cpp \
			widget.cpp \
    pcaprojector.cpp

OTHER_FILES += \
    plugin.json




