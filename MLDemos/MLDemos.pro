# ##########################
# Configuration      #
# ##########################
TEMPLATE = app
QT += opengl svg

TARGET = mldemos
NAME = mldemos
MLPATH =..
DESTDIR = $$MLPATH

macx:ICON = logo.icns
win32:RC_FILE = MachineLearning.rc
RESOURCES += mldemos.qrc

CONFIG += mainApp
include($$MLPATH/MLDemos_variables.pri)


# ##########################
# Source Files       #
# ##########################
FORMS += aboutDialog.ui \
	algorithmOptions.ui \
	optsClassify.ui \
	optsCluster.ui \
	optsRegress.ui \
    optsMaximize.ui \
    optsReinforcement.ui \
    optsDynamic.ui \
    optsProject.ui \
    optsCompare.ui \
	mldemos.ui \
	drawingTools.ui \
	drawingToolsContext1.ui \
	drawingToolsContext2.ui \
	drawingToolsContext3.ui \
	drawingToolsContext4.ui \
	statisticsDialog.ui \
	viewOptions.ui \
    compare.ui \
    expose.ui \
    manualSelection.ui \
    inputDimensions.ui \
    datagenerator.ui \
    gridsearch.ui

HEADERS += basicMath.h \
    canvas.h \
    datasetManager.h \
	optimization_test_functions.h \
	gettimeofday.h \
	drawUtils.h \
	drawSVG.h \
	drawTimer.h \
    mldemos.h \
    mymaths.h \
    public.h \
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
    compare.h \
    spline.h \
    datagenerator.h \
    gridsearch.h

SOURCES += \
	main.cpp \
	mldemos.cpp \
    mlprocessing.cpp \
    mlstats.cpp \
	mlsaving.cpp \
    compare.cpp \
    mltrain.cpp \
    datagenerator.cpp \
    gridsearch.cpp

OTHER_FILES += \
    shaders/drawSamples.fsh \
    shaders/drawSamples.vsh \
    shaders/smoothTransparent.fsh \
    shaders/smoothTransparent.vsh \
    shaders/renderFBO.fsh \
    shaders/renderFBO.vsh \
    shaders/blurFBO.fsh \
    shaders/blurFBO.vsh
