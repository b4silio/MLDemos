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
#ifndef _GPR_H_
#define _GPR_H_

#include <vector>
#include "SOGP.h"
#include "basicMath.h"

class GPR
{
private:
	int dim;
	SOGP *sogp;
	bool bTrained;
	std::vector<cvVec2> bv;
	double param1, param2;
	int kernelType;
	int degree;

public:
	bool bShowBasis;
	GPR() : sogp(0), dim(1), kernelType(kerRBF), bTrained(false), param1(1), param2(0.1), bShowBasis(false), degree(1){};
	void Draw(IplImage *display);
	void Train(std::vector<float *> inputs, std::vector<float> outputs, u32 dim);
	float Test(float *sample, u32 dim);
	void SetParams(double p1, double p2, int kType, int d=1){param1=p1; param2=p2; kernelType=kType; degree = d;};
	void Clear();
};

#endif // _GPR_H_
