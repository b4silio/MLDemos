
##############################
#                            #
#     MLDemos Variables      #
#                            #
##############################

# PLEASE EDIT THIS PART TO FIT YOUR NEEDS/SETUP
QT += svg opengl
greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
	macx: LIBS += -framework QtWidgets
} else {
}
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#macx: LIBS += -framework QtWidgets

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
    CONFIG(release){
        LIBS += -lQtSvg4 -lQtOpenGL4 -lQtGui4 -lQtCore4
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
    CONFIG += opencv22
    OPENCV_VER = 230
}else:macx{
#    CONFIG += opencv21
    CONFIG += opencv22
}else{
    CONFIG += opencv$$system(pkg-config --modversion opencv | cut -d . -f'1,2' | sed -e \'s/\.[2-9]/2/g\' -e \'s/\.1/1/g\')
#   CONFIG += opencv22
#   CONFIG += opencv21
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
    CONFIG(boost):BOOST = E:/DEV/boost_1_47_0
    CONFIG(opencv21)|CONFIG(opencv22):OPENCV = C:/DEV/OpenCV2.3-GCC
}else:macx{
	CONFIG(boost):BOOST = /usr/local/include
    CONFIG(opencv22|opencv21):OPENCV = /usr/local/opencv
}

# PLEASE EDIT UNTIL HERE TO FIT YOUR NEEDS/SETUP

##########################
# Autoconfiguration part #
##########################

win32{
	DEFINES += WIN32
	CONFIG += WIN32
}else:macx{
    DEFINES += MACX
    CONFIG += MACX
}else:unix{
    CONFIG += link_pkgconfig
}

win32{
	#INCLUDEPATH += C:/DEV/glew-1.9.0/include
	#LIBS += -LC:/DEV/glew-1.9.0/lib
	#LIBS += -lglew32 -lglu32 -lopengl32 -lglut32win
}

# OPENCV
win32:CONFIG(opencv22){
    DEFINES += OPENCV22
    INCLUDEPATH += . "$$OPENCV/include"
    LIBS += -L"$$OPENCV/lib"
    LIBS += -L"$$OPENCV/bin"
#    INCLUDEPATH += . "$$OPENCV/build/include"
#	LIBS += -L"$$OPENCV/build/x86/mingw/lib"
#	LIBS += -L"$$OPENCV/build/x86/mingw/bin"
    LIBS += -lopencv_core$$OPENCV_VER \
        -lopencv_features2d$$OPENCV_VER \
		-lopencv_highgui$$OPENCV_VER \
		-lopencv_imgproc$$OPENCV_VER \
		-lopencv_legacy$$OPENCV_VER \
                -lopencv_ml$$OPENCV_VER
}
macx{
    CONFIG(opencv22){
        DEFINES += OPENCV22
		LIBS += -L/usr/local/lib
        message("Using opencv22 or later")
        LIBS += -lopencv_core \
                -lopencv_calib3d \
                -lopencv_contrib \
                -lopencv_features2d \
                -lopencv_flann \
                -lopencv_gpu \
                -lopencv_highgui \
                -lopencv_imgproc \
                -lopencv_legacy \
                -lopencv_objdetect \
                -lopencv_video \
                -lopencv_ml

    }
    CONFIG(opencv21) {
        DEFINES += OPENCV21
        message("Using opencv21")
        LIBS += -lcv \
                -lcxcore \
                -lcvaux \
                -lml \
                -lhighgui
    }
}else:unix{
# some issues between qmake and pkgconfig
# invoking pkg-config manually instead
    CONFIG(opencv22){
        PKGCONFIG += opencv
        DEFINES += OPENCV22
        message("Using opencv22 or later")
        LIBS += $$system(pkg-config --libs opencv)
    }
    CONFIG(opencv21) {
	PKGCONFIG += opencv
	DEFINES += OPENCV21
        message("Using opencv21")
        LIBS += $$system(pkg-config --libs opencv)
    }
}

# BOOST
CONFIG(boost){
    DEFINES += WITHBOOST
    message("Using boost libraries")
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
	message("debug mode")
#	QMAKE_CXXFLAGS += -pg
#	QMAKE_LFLAGS += -pg
}else{
	message("release mode")
#	linux-g++:QMAKE_CXXFLAGS += -O2 -march=native -pipe
#        macx-g++:QMAKE_CXXFLAGS += -O2
#        win32-g++:QMAKE_CXXFLAGS += -O2
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

#QMAKE_CXXFLAGS += -pg
#QMAKE_CXXFLAGS_DEBUG += -pg
#QMAKE_LFLAGS += -pg
#QMAKE_LFLAGS_DEBUG += -pg


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
