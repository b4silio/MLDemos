# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_Reinforcement
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
FORMS += paramsRandom.ui paramsGA.ui paramsPower.ui paramsDP.ui
# paramsGA.ui paramsParticles.ui \
# paramsNLopt.ui paramsParticleFilters.ui
HEADERS +=	\
			canvas.h \
            datasetManager.h \
            mymaths.h \
            drawUtils.h \
            optimization_test_functions.h \
#			maximizePower.h \
#			maximizeParticles.h \
#			maximizeSwarm.h \
#			maximizeGradient.h \
#			maximizeDonut.h \
#           interfaceParticles.h \
#           interfaceParticleFilters.h \
            gaPeon.h \
            gaTrainer.h \
#			mvnpdf.h \
#           maximizeNLopt.h \
#           interfaceNLopt.h \
            reinforcementPower.h \
            reinforcementRandom.h \
            reinforcementGA.h \
            reinforcementDP.h \
            interfaceRandom.h \
            interfacePower.h \
            interfaceDP.h \
            interfaceGA.h \
            pluginReinforcements.h

SOURCES += 	\
#			maximizeParticles.cpp \
#			maximizeSwarm.cpp \
#			maximizeGradient.cpp \
#			maximizeDonut.cpp \
#			interfaceParticles.cpp \
#           interfaceParticleFilters.cpp \
            gaPeon.cpp \
            gaTrainer.cpp \
#           maximizeNLopt.cpp \
#           interfaceNLopt.cpp \
            reinforcementPower.cpp \
            reinforcementRandom.cpp \
            reinforcementGA.cpp \
            reinforcementDP.cpp \
            interfaceRandom.cpp \
            interfacePower.cpp \
            interfaceDP.cpp \
            interfaceGA.cpp \
            pluginReinforcements.cpp

###########################
# Dependencies            #
###########################
#HEADERS += \
#	PSO/pso.h \
#	PSO/memoryAllocation.h \
#	PSO/optimizer.h

HEADERS +=	\
	fgmm/em.h \
	fgmm/fgmm++.hpp \
	fgmm/fgmm.h \
	fgmm/gaussian.h \
	fgmm/regression.h \
    fgmm/smat.h

#HEADERS +=	\
#	newmat11/controlw.h \
#	newmat11/include.h \
#	newmat11/myexcept.h \
#	newmat11/newmat.h \
#	newmat11/newmatap.h \
#	newmat11/newmatio.h \
#	newmat11/newmatnl.h \
#	newmat11/newmatrc.h \
#	newmat11/newmatrm.h \
#	newmat11/precisio.h \
#	newmat11/solution.h

OTHER_FILES += \
    plugin.json
