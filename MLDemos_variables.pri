
##############################
#                            #
#     MLDemos Variables      #
#                            #
##############################

# PLEASE EDIT THIS PART TO FIT YOUR NEEDS/SETUP
QT += svg opengl openglwidgets
QT += widgets
macx: LIBS += -framework QtWidgets

macx {
    DEFINES += MACX
    CONFIG += MACX

    QMAKE_APPLE_DEVICE_ARCHS="x86_64"
    QMAKE_MACOSX_DEPLOYMENT_TARGET=12.0
    QMAKE_MAC_SDK=macosx
}
CONFIG += c++14

#################################
#         Project Paths         #
#################################
CONFIG(coreLib)|CONFIG(mainApp) ROOT = ..
else: ROOT = ../..

!CONFIG(coreLib){
    INCLUDEPATH += $$ROOT
    INCLUDEPATH += $$ROOT/Core
    INCLUDEPATH += $$OUT_PWD/$$ROOT/Core
    INCLUDEPATH += $$ROOT/_3rdParty
    INCLUDEPATH += $$OUT_PWD/$$ROOT/_3rdParty
    LIBS += -L$$ROOT/lib
    PRE_TARGETDEPS += $$OUT_PWD/$$ROOT/Core
}

#CONFIG(coreLib): DESTDIR=$$ROOT/lib
#else: DESTDIR=$$ROOT/bin

DEPENDPATH += . \
		.. \
		$${MLPATH}/Core \
		$${MLPATH}/_3rdParty
INCLUDEPATH += . \
		$${MLPATH}/Core \
		$${MLPATH}/MLDemos \
		$${MLPATH}/_3rdParty

win32{
    LIBS += -lopengl32
    CONFIG(release){
#        LIBS += -lQtSvg4 -lQtOpenGL4 -lQtGui4 -lQtCore4
    }else{
        LIBS += -lQtSvgd4 -lQtOpenGLd4 -lQtGuid4 -lQtCored4
    }
}
CONFIG(coreLib){
}else{
    LIBS += -L$$MLPATH -L$$MLPATH/Core -lCore
}
LIBS += -L$$MLPATH/_3rdParty -l3rdParty


########################################
# Location of intermediate build files #
########################################
win32:MLBUILD = C:/tmp/MLDemos/$$NAME
unix:MLBUILD = build

######################
# Optional libraries #
######################
# OpenCV : We currently support 2.1 and upwards (on windows 2.2 upwards).
# You may select which version you have
# or let qmake find it out for you (linux only).

win32{
    CONFIG += opencv3
    OPENCV_VER = 420
}else:macx{
    CONFIG += opencv3
}else{
    CONFIG += opencv$$system(pkg-config --modversion opencv | cut -d . -f'1,2' | sed -e \'s/\.[2-9]/2/g\' -e \'s/\.1/1/g\')
}

# Boost
    CONFIG += boost

############################################
# PATHS for the BOOST and OPENCV libraries #
############################################
# Give here the library and include paths if they are non-standard
# (like /usr/local/[lib|include] for instance).
# For windows, please specifiy it. For linux (macosx?) the libraries
# are autodetected provided you have installed using your package
# manager     CONFIG(opencv21)|CONFIG(opencv22):OPENCV = E:/Dvt/opencv/build/x86/mingw

win32{
    BOOST = C:/DEV/boost_1_66_0
    OPENCV = C:/opencv/build
}else:macx{
    BOOST = /usr/local/include
    BREW_PATH = /usr/local/Cellar

    BREW_FFMPEG = $$BREW_PATH/ffmpeg/7.1_3
    BREW_OPENCV = $$BREW_PATH/opencv/4.10.0_12
    BREW_LIBPNG = $$BREW_PATH/libpng/1.6.44
    BREW_PNGPP  = $$BREW_PATH/png++/0.2.10
    BREW_EIGEN  = $$BREW_PATH/eigen/3.4.0_1
    BREW_GSL    = $$BREW_PATH//gsl/2.8
}


# PLEASE EDIT UNTIL HERE TO FIT YOUR NEEDS/SETUP

##########################
# Autoconfiguration part #
##########################


# OPENCV
win32{
    DEFINES += OPENCV3
    INCLUDEPATH += . "$$OPENCV/include"
    LIBS += -L"$$OPENCV/install/x86/mingw/lib"
    LIBS += -L"$$OPENCV/install/x86/mingw/bin"
#    INCLUDEPATH += . "$$OPENCV/build/include"
#	LIBS += -L"$$OPENCV/build/x86/mingw/lib"
#	LIBS += -L"$$OPENCV/build/x86/mingw/bin"
        LIBS += \
        #-lopencv_bgsegm$$OPENCV_VER \
        -lopencv_calib3d$$OPENCV_VER \
        -lopencv_highgui$$OPENCV_VER \
        -lopencv_imgcodecs$$OPENCV_VER \
        -lopencv_imgproc$$OPENCV_VER \
        -lopencv_videoio$$OPENCV_VER \
        -lopencv_objdetect$$OPENCV_VER \
        -lopencv_ml$$OPENCV_VER \
        -lopencv_features2d$$OPENCV_VER \
        -lopencv_core$$OPENCV_VER
}
macx{
    DEFINES += OPENCV3

    INCLUDEPATH += $$BREW_FFMPEG/include
    INCLUDEPATH += $$BREW_OPENCV/include/opencv4
    INCLUDEPATH += $$BREW_LIBPNG/include
    INCLUDEPATH += $$BREW_PNGPP/include
    INCLUDEPATH += $$BREW_EIGEN/include/eigen3
    INCLUDEPATH += $$BREW_GSL/include

    LIBS += -L$$BREW_FFMPEG/lib
    LIBS += -L$$BREW_OPENCV/lib
    LIBS += -L$$BREW_GSL/lib
    LIBS += \
    -lopencv_bgsegm \
    -lopencv_calib3d \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_imgproc \
    -lopencv_videoio \
    -lopencv_objdetect \
    -lopencv_ml \
    -lopencv_core
    LIBS += -lgsl
}else:unix{
# some issues between qmake and pkgconfig
# invoking pkg-config manually instead
    CONFIG(opencv22){
        PKGCONFIG += opencv
        DEFINES += OPENCV22
        #message("Using opencv22 or later")
        LIBS += $$system(pkg-config --libs opencv)
    }
    CONFIG(opencv21) {
        PKGCONFIG += opencv
        DEFINES += OPENCV21
        #message("Using opencv21")
        LIBS += $$system(pkg-config --libs opencv)
    }
}

win32{
    DEFINES += WIN32
    CONFIG += WIN32
}else:macx{
    DEFINES += MACX
    CONFIG += MACX
}else:unix{
    CONFIG += link_pkgconfig
}

# BOOST
CONFIG(boost){
    DEFINES += WITHBOOST
    #message("Using boost libraries")
    macx|win32{
        INCLUDEPATH += "$$BOOST"
    }else:unix{
        #PKGCONFIG += boost # Boost doesn't provide its own pc file yet...
    }
}

###############
# Misc. stuff #
###############

mainApp|coreLib{
}else{
	TARGET = $$qtLibraryTarget($$NAME)
        CONFIG(debug, debug|release):DESTDIR = "$$MLPATH/pluginsDebug"
        CONFIG(release, debug|release):DESTDIR = "$$MLPATH/plugins"
}
CONFIG(debug, debug|release){
	DEFINES += DEBUG
}

win32{
	CONFIG(Debug, Debug|Release){
        MOC_DIR = $${MLBUILD}/Debug
        UI_DIR = $${MLBUILD}/Debug
        RCC_DIR = $${MLBUILD}/Debug
        OBJECTS_DIR = $${MLBUILD}/Debug
	}else{
        MOC_DIR = $${MLBUILD}/Release
        UI_DIR = $${MLBUILD}/Release
        RCC_DIR = $${MLBUILD}/Release
        OBJECTS_DIR = $${MLBUILD}/Release
	}
}else{
    MOC_DIR = $${MLBUILD}
    UI_DIR = $${MLBUILD}
    RCC_DIR = $${MLBUILD}
    OBJECTS_DIR = $${MLBUILD}
}

################################
# Turn the bloody warnings off #
################################
win32-g++|macx|unix {
	QMAKE_CXXFLAGS_WARN_ON = ""
	QMAKE_CXXFLAGS += -Wno-all
	#QMAKE_CXXFLAGS += -Wno-endif-labels
	QMAKE_CXXFLAGS += -Wno-unused-variable
	QMAKE_CXXFLAGS += -Wno-unused-parameter
	#QMAKE_CXXFLAGS += -Wno-switch
	QMAKE_CXXFLAGS += -Wtrigraphs
	QMAKE_CXXFLAGS += -Wreturn-type
	QMAKE_CXXFLAGS += -Wnon-virtual-dtor
	#QMAKE_CXXFLAGS += -Woverloaded-virtual
	#QMAKE_CXXFLAGS += -Wunused-variable
	QMAKE_CXXFLAGS += -Wunused-value
	QMAKE_CXXFLAGS += -Wunknown-pragmas
	QMAKE_CXXFLAGS += -Wno-shadow
	#QMAKE_CXXFLAGS += -Wno-deprecated-declarations
	#QMAKE_CXXFLAGS += -Wno-missing-braces
}
