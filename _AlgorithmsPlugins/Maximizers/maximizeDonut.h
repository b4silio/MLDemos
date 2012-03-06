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
#ifndef _MAXIMIZE_DONUT_H_
#define _MAXIMIZE_DONUT_H_

#include <vector>
#include "maximize.h"

class MaximizeDonut: public Maximizer
{
private:
	int age;
	int maxAge;
	float variance;
	float fingerprint;
	int k;
	std::vector< std::pair<double, std::pair<fvec, fvec> > > best;
	fvec lastSigma;
	bool bAdaptive;

	fvec Generate(fvec point, fvec sigma, float uniform=false);
	fvec GetBestMean();
	fvec GetBestSigma(fvec mean);
	QImage DrawMap();

public:
	MaximizeDonut();
	~MaximizeDonut();

	void SetParams(int k, float variance=0.2, bool bAdaptive=false);
	void Draw(QPainter &painter);
	void Train(float *dataMap, fVec size, fvec startingPoint=fvec());
	fvec Test( const fvec &sample);
	fvec Test(const fVec &sample);
    const char *GetInfoString();
};

#endif // _MAXIMIZE_DONUT_H_
