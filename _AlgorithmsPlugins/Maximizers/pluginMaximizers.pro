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
FORMS += paramsMaximizers.ui paramsGA.ui paramsParticles.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			$$MLDEMOS/drawUtils.h \
			maximizeRandom.h \
			maximizePower.h \
			maximizeGA.h \
			maximizeParticles.h \
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
			maximizeGradient.cpp \
			maximizeDonut.cpp \
			pluginMaximizers.cpp \
			interfaceGA.cpp \
			interfaceParticles.cpp \
			gaPeon.cpp \
			gaTrainer.cpp \
			interfaceBasic.cpp

###########################
# Dependencies            #
###########################
HEADERS +=	\
	fgmm/em.h \
	fgmm/fgmm++.hpp \
	fgmm/fgmm.h \
	fgmm/gaussian.h \
	fgmm/regression.h \
	fgmm/smat.h

SOURCES += \
	fgmm/em.cpp \
	fgmm/gaussian.cpp \
	fgmm/gmm.cpp \
	fgmm/gmmregression.cpp \
	fgmm/smat.cpp \
	fgmm/update.cpp

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
SOURCES += \
	newmat11/bandmat.cpp \
	newmat11/cholesky.cpp \
	newmat11/evalue.cpp \
	newmat11/fft.cpp \
	newmat11/hholder.cpp \
	newmat11/jacobi.cpp \
	newmat11/myexcept.cpp \
	newmat11/newfft.cpp \
	newmat11/newmat1.cpp \
	newmat11/newmat2.cpp \
	newmat11/newmat3.cpp \
	newmat11/newmat4.cpp \
	newmat11/newmat5.cpp \
	newmat11/newmat6.cpp \
	newmat11/newmat7.cpp \
	newmat11/newmat8.cpp \
	newmat11/newmat9.cpp \
	newmat11/newmatex.cpp \
	newmat11/newmatnl.cpp \
	newmat11/newmatrm.cpp \
	newmat11/nm_misc.cpp \
	newmat11/solution.cpp \
	newmat11/sort.cpp \
	newmat11/submat.cpp \
	newmat11/svd.cpp
