# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_Maximizers
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
#QMAKE_CXXFLAGS += -fopenmp
#LIBS += -lgomp
LIBS += -L$$MLPATH/_3rdParty -l3rdParty
###########################
# Source Files            #
###########################
FORMS += paramsMaximizers.ui paramsGA.ui paramsParticles.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			$$MLDEMOS/drawUtils.h \
			$$MLDEMOS/optimization_test_functions.h \
			maximizeRandom.h \
			maximizePower.h \
			maximizeGA.h \
			maximizeParticles.h \
			maximizeSwarm.h \
			maximizeGradient.h \
			maximizeDonut.h \
			pluginMaximizers.h \
			interfaceParticles.h \
			interfaceGA.h \
			gaPeon.h \
			gaTrainer.h \
			mvnpdf.h \
			interfaceBasic.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			$$MLDEMOS/drawUtils.cpp \
			maximizeRandom.cpp \
			maximizePower.cpp \
			maximizeGA.cpp \
			maximizeParticles.cpp \
			maximizeSwarm.cpp \
			maximizeGradient.cpp \
			maximizeDonut.cpp \
			pluginMaximizers.cpp \
			interfaceGA.cpp \
			interfaceParticles.cpp \
			gaPeon.cpp \
			gaTrainer.cpp \
			interfaceBasic.cpp \

###########################
# Dependencies            #
###########################
HEADERS += \
	PSO/pso.h \
	PSO/memoryAllocation.h \
	PSO/optimizer.h

HEADERS +=	\
	fgmm/em.h \
	fgmm/fgmm++.hpp \
	fgmm/fgmm.h \
	fgmm/gaussian.h \
	fgmm/regression.h \
	fgmm/smat.h

HEADERS +=	\
	newmat11/controlw.h \
	newmat11/include.h \
	newmat11/myexcept.h \
	newmat11/newmat.h \
	newmat11/newmatap.h \
	newmat11/newmatio.h \
	newmat11/newmatnl.h \
	newmat11/newmatrc.h \
	newmat11/newmatrm.h \
	newmat11/precisio.h \
	newmat11/solution.h
