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

#ifndef SPATIAL_FORCE_H
#define SPATIAL_FORCE_H

#include "SpatialVector.h"
#ifdef USE_MATHLIB_NAMESPACE
namespace MathLib {
#endif


/**
 * \class SpatialForce
 * 
 * \ingroup MathLib
 * 
 * \brief An implementation of the template TVector class for Spatial Vector notation
 */
class SpatialForce : public SpatialVector
{
  
public:
  /// Empty constructor
  inline SpatialForce():SpatialVector(){};
  /// Copy constructor
  inline SpatialForce(const SpatialForce &vector):SpatialVector(vector){};
  /// Copy constructor
  inline SpatialForce(const SpatialVector &vector):SpatialVector(vector){};
  /// Data-based constructor
  inline SpatialForce(REALTYPE wx, REALTYPE wy, REALTYPE wz, REALTYPE x, REALTYPE y, REALTYPE z):SpatialVector(wx,wy,wz,x,y,z){}
  /// Vector component-based constructor
  inline SpatialForce(const Vector3& angular, const Vector3& linear):SpatialVector(angular,linear){} 
  /// Destructor
  inline ~SpatialForce(){};




};

#ifdef USE_MATHLIB_NAMESPACE
}
#endif

#endif
