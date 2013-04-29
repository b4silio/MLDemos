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
			canvas.h \
			datasetManager.h \
			mymaths.h \
			drawUtils.h \
			SEDS.h \
			dynamicalSEDS.h \
			interfaceSEDSDynamic.h

SOURCES += 	\
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
	MathLib/Vector3.h \
	nlopt.hpp \
	nlopt/tools.h \
	nlopt/stogo_config.h \
	nlopt/stogo.h \
	nlopt/soboldata.h \
	nlopt/slsqp.h \
	nlopt/redblack.h \
	nlopt/praxis.h \
	nlopt/nlopt-util.h \
	nlopt/nlopt-internal.h \
	nlopt/nlopt-in.hpp \
	nlopt/nlopt_optimize_usage.h \
	nlopt/nlopt.hpp \
	nlopt/nlopt.h \
	nlopt/newuoa.h \
	nlopt/neldermead.h \
	nlopt/mma.h \
	nlopt/mlsl.h \
	nlopt/luksan.h \
	nlopt/local.h \
	nlopt/linalg.h \
	nlopt/isres.h \
	nlopt/global.h \
	nlopt/f77funcs_.h \
	nlopt/f77funcs.h \
	nlopt/direct-internal.h \
	nlopt/direct.h \
	nlopt/crs.h \
	nlopt/cobyla.h \
	nlopt/cdirect.h \
	nlopt/bobyqa.h \
	nlopt/auglag.h \
	nlopt/config.h

OTHER_FILES += \
    plugin.json
