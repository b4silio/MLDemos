
##############################
#                            #
#     MLDemos Variables      #
#                            #
##############################

# PLEASE EDIT THIS PART TO FIT YOUR NEEDS/SETUP

########################################
# Location of intermediate build files #
########################################
win32:MLBUILD = C:/tmp/MLDemos/$$NAME
unix:MLBUILD = /tmp/MLDemos/$$NAME

######################
# Optional libraries #
######################
# OpenCV : We currently support 2.1 and upwards. You may select which version
# you have or let qmake find it out for you.

win32{
	CONFIG += opencv22
#	CONFIG += opencv21
	OPENCV_VER = 231
}else{
#	CONFIG += opencv$$system(pkg-config --modversion opencv | cut -d . -f'1,2' | sed -e \'s/\.[2-9]/2/g\' -e \'s/\.1/1/g\')
    CONFIG += opencv22
#	CONFIG += opencv21
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
# manager

win32{
	CONFIG(boost):BOOST = E:/DEV/boost_1_47_0
	CONFIG(opencv22|opencv21):OPENCV = C:/DEV/OpenCV2.3-GCC
}else:macx{
    CONFIG(boost):BOOST = /usr/local/boost_1_47_0
    CONFIG(opencv22|opencv21):OPENCV = /usr/local/opencv
}

#	INCLUDEPATH +=	/usr/include/qt4 \
#			/usr/include/qt4/QtCore \
#			/usr/include/qt4/QtGui \
#			/usr/include/qt4/QtSvg \
#			/usr/include/qt4/QtOpenGL
#	LIBS += -L/usr/local/lib


# PLEASE EDIT UNTIL HERE TO FIT YOUR NEEDS/SETUP

##########################
# Autoconfiguration part #
##########################

win32{
	DEFINES += WIN32
	CONFIG += WIN32
}unix{
	macx:DEFINES += MACX
	CONFIG += link_pkgconfig
}

# OPENCV
win32:CONFIG(opencv22){
	INCLUDEPATH += . "$$OPENCV/include/"
	LIBS += -L"$$OPENCV/lib/"
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
        message("Using opencv22 or later")
        LIBS += -lopencv_core \
                -lopencv_features2d \
                -lopencv_highgui \
                -lopencv_imgproc \
                -lopencv_legacy \
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
    macx{
        INCLUDEPATH += "$$BOOST"
    }else:unix{
        #PKGCONFIG += boost # Boost doesn't provide its own pc file yet...
    }else:win32{
        INCLUDEPATH += "$$BOOST/include"
    }
}

# QT
unix{
#	PKGCONFIG = QtCore QtGui QtSvg QtOpenGL
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
}else{
	message("release mode")
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
	MOC_DIR = $${MLBUILD}/build
	UI_DIR = $${MLBUILD}/build
	RCC_DIR = $${MLBUILD}/build
	OBJECTS_DIR = $${MLBUILD}/build
}

DEPENDPATH += . \
		.. \
		$${MLPATH}/Core \
		$${MLPATH}/_3rdParty
INCLUDEPATH += . \
		$${MLPATH}/Core \
		$${MLPATH}/MLDemos \
		$${MLPATH}/_3rdParty

CONFIG(coreLib){
}else{
	LIBS += -L$$MLPATH/Core -lCore
}
LIBS += -L$$MLPATH/_3rdParty -l3rdParty

################################
# Turn the bloody warnings off #
################################
win32-g++|macx|unix {
	QMAKE_CXXFLAGS_WARN_ON = ""
	QMAKE_CXXFLAGS += -Wno-all
	QMAKE_CXXFLAGS += -Wno-endif-labels
	QMAKE_CXXFLAGS += -Wno-unused-variable
	QMAKE_CXXFLAGS += -Wno-unused-parameter
	QMAKE_CXXFLAGS += -Wno-switch
	QMAKE_CXXFLAGS += -Wtrigraphs
	QMAKE_CXXFLAGS += -Wreturn-type
	#QMAKE_CXXFLAGS += -Wnon-virtual-dtor
    #QMAKE_CXXFLAGS += -Woverloaded-virtual
	#QMAKE_CXXFLAGS += -Wunused-variable
	#QMAKE_CXXFLAGS += -Wunused-value
	QMAKE_CXXFLAGS += -Wunknown-pragmas
	QMAKE_CXXFLAGS += -Wno-shadow
	QMAKE_CXXFLAGS += -Wno-deprecated-declarations
	QMAKE_CXXFLAGS += -Wno-missing-braces
}
