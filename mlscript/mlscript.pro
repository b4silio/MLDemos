# ##########################
# Configuration      #
# ##########################
TEMPLATE = app

TARGET = mlscript
NAME = mlscript
MLPATH =..
DESTDIR = $$MLPATH

CONFIG += mainApp
include($$MLPATH/MLDemos_variables.pri)


# ##########################
# Source Files       #
# ##########################

HEADERS += basicMath.h \
    canvas.h \
    datasetManager.h \
	gettimeofday.h \
    mymaths.h \
    public.h \
	roc.h \
    types.h \
    interfaces.h \
	classifier.h \
	obstacles.h \
	regressor.h \
    maximize.h \
    reinforcement.h \
    dynamical.h \
    clusterer.h

SOURCES += \
    main.cpp
