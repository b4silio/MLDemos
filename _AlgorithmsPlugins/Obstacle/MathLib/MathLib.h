/*
 * Copyright (C) 2010 Learning Algorithms and Systems Laboratory, EPFL, Switzerland
 * Author: Eric Sauser
 * email:   eric.sauser@a3.epf.ch
 * website: lasa.epfl.ch
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#ifndef __MATHLIB_H__
#define __MATHLIB_H__


#define  USE_T_EXTENSIONS

#include "MathLibCommon.h"
#include "Macros.h"

#include "Vector.h"

#ifdef  USE_T_EXTENSIONS
#include "TVector.h"
#include "Vector3.h"
#endif


#include "Matrix.h"

#include "SpatialVector.h"
#include "SpatialMatrix.h"
#include "SpatialFrame.h"
#include "SpatialForce.h"
#include "SpatialInertia.h"
#include "SpatialVelocity.h"

#ifdef USE_T_EXTENSIONS
#include "TMatrix.h"
#include "Matrix3.h"
#include "Matrix4.h"

#include "ReferenceFrame.h"
#endif

#include "Differentiator.h"
#include "Regression.h"

#endif

