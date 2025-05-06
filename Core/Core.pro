###########################
# Configuration           #
###########################
TEMPLATE = lib
QT += svg opengl
NAME = Core
MLPATH =..
win32:CONFIG += coreLib
else:CONFIG += coreLib static

include($$MLPATH/MLDemos_variables.pri)

win32{
	DESTDIR = ..
}
###########################
# Files                   #
###########################
FORMS += expose.ui \
    dataImport.ui
HEADERS += \
	basicMath.h \
	canvas.h \
	datasetManager.h \
	optimization_test_functions.h \
	gettimeofday.h \
	drawUtils.h \
	drawSVG.h \
	drawTimer.h \
    mymaths.h \
    expose.h \
    public.h \
    parser.h \
	roc.h \
	types.h \
	widget.h \
	interfaces.h \
	classifier.h \
	obstacles.h \
	regressor.h \
    maximize.h \
    reinforcement.h \
    dynamical.h \
	clusterer.h \
	fileUtils.h \
	spline.h \
    projector.h \
    dataImporter.h \
    contours.h \
    qcontour.h \
    animationlabel.h \
    reinforcementProblem.h \
    kmeans.h \
    glwidget.h \
    glUtils.h

SOURCES += \
	canvas.cpp \
	datasetManager.cpp \
	drawUtils.cpp \
	drawSVG.cpp \
	drawTimer.cpp \
	mymaths.cpp \
    expose.cpp \
    roc.cpp \
	fileUtils.cpp \
    parser.cpp \
    widget.cpp \
    dataImporter.cpp \
    contours.cpp \
    qcontour.cpp \
    animationlabel.cpp \
    reinforcementProblem.cpp \
    kmeans.cpp \
    glwidget.cpp \
    glUtils.cpp \
    clusterer.cpp \
    canvas-drawing.cpp \
    canvas-interaction.cpp

RESOURCES +=

DISTFILES +=
