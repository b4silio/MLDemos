# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_DSAvoid
MLPATH =../..

include("$$MLPATH/MLDemos_variables.pri")
###########################
# Source Files            #
###########################
HEADERS +=	\
			canvas.h \
			datasetManager.h \
			mymaths.h \
			interfaces.h \
			obstacles.h \
			DSAvoid.h \
			interfaceAvoidance.h

SOURCES += 	\
			DSAvoid.cpp \
			interfaceAvoidance.cpp

###########################
# Dependencies            #
###########################
HEADERS +=	\
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
