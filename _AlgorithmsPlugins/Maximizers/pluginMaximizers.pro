# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_Maximizers
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
FORMS += paramsMaximizers.ui paramsGA.ui paramsParticles.ui \
    paramsNLopt.ui paramsParticleFilters.ui
HEADERS +=	\
			canvas.h \
			datasetManager.h \
			mymaths.h \
			drawUtils.h \
			optimization_test_functions.h \
			maximizeRandom.h \
			maximizePower.h \
			maximizeGA.h \
			maximizeParticles.h \
			maximizeSwarm.h \
			maximizeGradient.h \
			maximizeDonut.h \
			pluginMaximizers.h \
            interfaceParticles.h \
            interfaceParticleFilters.h \
            interfaceGA.h \
			gaPeon.h \
			gaTrainer.h \
			mvnpdf.h \
			interfaceBasic.h \
    maximizeNLopt.h \
    interfaceNLopt.h

SOURCES += 	\
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
            interfaceParticleFilters.cpp \
            gaPeon.cpp \
			gaTrainer.cpp \
			interfaceBasic.cpp \
    maximizeNLopt.cpp \
    interfaceNLopt.cpp

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

OTHER_FILES += \
    plugin.json
