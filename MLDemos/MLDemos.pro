# ##########################
# Configuration      #
# ##########################
TEMPLATE = app
QT -= network
TARGET = mldemos
NAME = mldemos
MLPATH =..
DESTDIR = $$MLPATH

#CONFIG(debug, debug|release) {
#	unix: TARGET = $${TARGET}_debug
#	else: TARGET = $${TARGET}d)
#}
macx:ICON = logo.icns
win32:RC_FILE = MachineLearning.rc
RESOURCES += mldemos.qrc

CONFIG += mainApp
include($$MLPATH/MLDemos_variables.pri)

macx:INCLUDEPATH += uiMac

###########################
#        Plugins          #
###########################
macx{
#	plugins.path = Contents/Resources/plugins
#	plugins.files = \
#	plugins/libIO_PCAFaces.dylib \
#	plugins/libIO_RandomEmitter.dylib \
#	plugins/libmld_GMM.dylib \
#	plugins/libmld_KernelMethods.dylib \
#	plugins/libmld_KNN.dylib \
#	plugins/libmld_LinearMethods.dylib \
#	plugins/libmld_LWPR.dylib \
#	plugins/libmld_SEDS.dylib \
#	plugins/libmld_DSAvoid.dylib

#	QMAKE_BUNDLE_DATA += plugins
}
# ##########################
# Source Files       #
# ##########################
macx{
FORMS += uiMac/aboutDialog.ui \
	uiMac/algorithmOptions.ui \
	uiMac/optsClassify.ui \
	uiMac/optsCluster.ui \
	uiMac/optsRegress.ui \
	uiMac/optsDynamic.ui \
	uiMac/mldemos.ui \
	uiMac/drawingTools.ui \
	uiMac/drawingToolsContext1.ui \
	uiMac/drawingToolsContext2.ui \
	uiMac/drawingToolsContext3.ui \
	uiMac/statisticsDialog.ui \
	uiMac/viewOptions.ui
} else {
FORMS += aboutDialog.ui \
	algorithmOptions.ui \
	optsClassify.ui \
	optsCluster.ui \
	optsRegress.ui \
	optsDynamic.ui \
	mldemos.ui \
	drawingTools.ui \
	drawingToolsContext1.ui \
	drawingToolsContext2.ui \
	drawingToolsContext3.ui \
	statisticsDialog.ui \
	viewOptions.ui
}
HEADERS += basicMath.h \
    basicOpenCV.h \
    canvas.h \
    datasetManager.h \
    gettimeofday.h \
    drawUtils.h \
    drawTimer.h \
    mldemos.h \
    mymaths.h \
    public.h \
    roc.h \
    sampleManager.h \
    types.h \
    widget.h \
    interfaces.h \
	classifier.h \
	obstacles.h \
	regressor.h \
    dynamical.h \
    clusterer.h
SOURCES += basicOpenCV.cpp \
    canvas.cpp \
    datasetManager.cpp \
    drawUtils.cpp \
    drawTimer.cpp \
    main.cpp \
    mldemos.cpp \
    mlprocessing.cpp \
    mlstats.cpp \
    mlsaving.cpp \
    mymaths.cpp \
    roc.cpp \
    sampleManager.cpp \
    widget.cpp

# ##########################
# Dependencies             #
# ##########################
HEADERS += fgmm/em.h \
    fgmm/fgmm++.hpp \
    fgmm/fgmm.h \
    fgmm/gaussian.h \
    fgmm/regression.h \
    fgmm/smat.h
SOURCES += fgmm/em.cpp \
    fgmm/gaussian.cpp \
    fgmm/gmm.cpp \
    fgmm/gmmregression.cpp \
    fgmm/smat.cpp \
    fgmm/update.cpp
