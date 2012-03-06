/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#ifndef _REGRESSOR_MLP_H_
#define _REGRESSOR_MLP_H_

#include <vector>
#include "regressor.h"
#include "basicOpenCV.h"

class RegressorMLP : public Regressor
{
private:
	u32 functionType; // 1: sigmoid, 2: gaussian
	u32 neuronCount;
	u32 layerCount;
	float alpha, beta;
	CvANN_MLP *mlp;
public:
	RegressorMLP();
	~RegressorMLP();
	void Train(std::vector< fvec > samples, ivec labels);
	fvec Test( const fvec &sample);
    const char *GetInfoString();

	void SetParams(u32 functionType, u32 neuronCount, u32 layerCount, f32 alpha, f32 beta);
};

#endif // _REGRESSOR_MLP_H_
