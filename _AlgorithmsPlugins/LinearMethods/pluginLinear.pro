# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_LinearMethods
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

###########################
# Source Files            #
###########################
FORMS += paramsLinear.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			classifierLinear.h \
			interfaceLinearClassifier.h \
			pluginLinear.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			classifierLinear.cpp \
			interfaceLinearClassifier.cpp \
			pluginLinear.cpp

###########################
# Dependencies            #
###########################
HEADERS += 	JnS/Matutil.h \
			JnS/JnS.h
SOURCES += 	JnS/Matutil.cpp \
			JnS/JnS.cpp
HEADERS += 	liblinear/linear.h \
			liblinear/tron.h \
			liblinear/blasp.h \
			liblinear/blas.h
SOURCES += 	liblinear/linear.cpp \
			liblinear/tron.cpp \
			liblinear/daxpy.c \
			liblinear/ddot.c \
			liblinear/dnrm2.c \
			liblinear/dscal.c
