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
#ifndef _MAXIMIZE_H_
#define _MAXIMIZE_H_

#include <vector>
#include "public.h"
#include "mymaths.h"
#include <QPainter>
#include "glUtils.h"

class Maximizer
{
protected:
	u32 dim;
	int w, h;
	bool bIterative;
	bool bConverged;
	fvec maximum;
	std::vector< fvec > visited;
	std::vector< fvec > history;
	std::vector<double> historyValue;
	double maximumValue;
	float *data;
	int evaluations;

public:
	int age, maxAge;
	double stopValue;

    Maximizer() : evaluations(0), stopValue(.99), maxAge(200), age(0), dim(2), bIterative(false) , bConverged(true), data(NULL), w(1), h(1), maximumValue(-FLT_MAX){ maximum.resize(2);}
    virtual ~Maximizer(){if(data) delete [] data;}
    void Maximize(float *dataMap, int w, int h) {Train(dataMap,fVec(w,h));}
    bool hasConverged(){return bConverged;}
    void SetConverged(bool converged){bConverged = converged;}
    std::vector<fvec> &History(){return history;}
    std::vector<double> &HistoryValue(){return historyValue;}
    fvec &Maximum(){return maximum;}
    double MaximumValue(){return GetValue(maximum);}
    std::vector<fvec> &Visited(){return visited;}
    int &Evaluations(){return evaluations;}
    static float GetValue(fvec sample, float *data, int w, int h)
    {
        int xIndex = max(0, min(w-1, (int)(sample[0]*w)));
        int yIndex = max(0, min(h-1, (int)(sample[1]*h)));
        int index = yIndex*w + xIndex;
        return data[index];
    }
    float GetValue(fvec sample)
	{
		int xIndex = max(0, min(w-1, (int)(sample[0]*w)));
		int yIndex = max(0, min(h-1, (int)(sample[1]*h)));
		int index = yIndex*w + xIndex;
		return data[index];
	}

    virtual void Draw(QPainter &painter){}
    virtual std::vector<GLObject> DrawGL(){return std::vector<GLObject>();}

    virtual void Train(float *dataMap, fVec size, fvec startingPoint=fvec()){}
    virtual fvec Test( const fvec &sample){ return fvec(); }
    virtual fvec Test(const fVec &sample){ return Test((fvec)sample); }
    virtual const char *GetInfoString(){return NULL;}
};

#endif // _MAXIMIZE_H_
