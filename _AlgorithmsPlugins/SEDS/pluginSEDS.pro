# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_SEDS
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

###########################
# Source Files            #
###########################
FORMS += paramsSEDS.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			$$MLDEMOS/basicOpenCV.h \
			$$MLDEMOS/drawUtils.h \
			SEDS.h \
			dynamicalSEDS.h \
			interfaceSEDSDynamic.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			$$MLDEMOS/basicOpenCV.cpp \
			$$MLDEMOS/drawUtils.cpp \
			SEDS.cpp \
			dynamicalSEDS.cpp \
			interfaceSEDSDynamic.cpp

###########################
# Dependencies            #
###########################
HEADERS +=	\
	fgmm/em.h \
	fgmm/fgmm++.hpp \
	fgmm/fgmm.h \
	fgmm/gaussian.h \
	fgmm/regression.h \
	fgmm/smat.h \
	MathLib/Differentiator.h \
	MathLib/GradientDescent.h \
	MathLib/IKGroupSolver.h \
	MathLib/IKSubSolver.h \
	MathLib/Macros.h \
	MathLib/MathLib.h \
	MathLib/MathLibCommon.h \
	MathLib/Matrix.h \
	MathLib/Matrix3.h \
	MathLib/Matrix4.h \
	MathLib/ReferenceFrame.h \
	MathLib/Regression.h \
	MathLib/SpatialForce.h \
	MathLib/SpatialFrame.h \
	MathLib/SpatialInertia.h \
	MathLib/SpatialMatrix.h \
	MathLib/SpatialVector.h \
	MathLib/SpatialVelocity.h \
	MathLib/TMatrix.h \
	MathLib/TVector.h \
	MathLib/Vector.h \
	MathLib/Vector3.h

SOURCES += \
	fgmm/em.cpp \
	fgmm/gaussian.cpp \
	fgmm/gmm.cpp \
	fgmm/gmmregression.cpp \
	fgmm/smat.cpp \
	fgmm/update.cpp \
	MathLib/Differentiator.cpp \
	MathLib/GradientDescent.cpp \
	MathLib/IKGroupSolver.cpp \
	MathLib/IKSubSolver.cpp \
	MathLib/Macros.cpp \
	MathLib/MathLib.cpp \
	MathLib/MathLibCommon.cpp \
	MathLib/Matrix.cpp \
	MathLib/Matrix3.cpp \
	MathLib/Matrix4.cpp \
	MathLib/ReferenceFrame.cpp \
	MathLib/Regression.cpp \
	MathLib/SpatialForce.cpp \
	MathLib/SpatialFrame.cpp \
	MathLib/SpatialInertia.cpp \
	MathLib/SpatialMatrix.cpp \
	MathLib/SpatialVector.cpp \
	MathLib/SpatialVelocity.cpp \
	MathLib/TMatrix.cpp \
	MathLib/TVector.cpp \
	MathLib/Vector.cpp \
	MathLib/Vector3.cpp
