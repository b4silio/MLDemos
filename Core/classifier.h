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
#ifndef _CLASSIFIER_H_
#define _CLASSIFIER_H_

#include <vector>
#include <map>
#include "public.h"
#include "roc.h"
#include "types.h"
#include "mymaths.h"

class Classifier
{
protected:
	std::vector< fvec > samples;
	ivec labels;
	u32 dim;
	u32 posClass;
	bool bSingleClass;
	bool bUsesDrawTimer;
	bool bMultiClass;

public:
    std::map<int,int> classMap, inverseMap;
    std::map<int,int> classes;
    std::vector<fvec> crossval;
	fvec fmeasures;
	std::vector< std::vector<f32pair> > rocdata;
	std::vector<const char *> roclabels;
    std::map<int, std::map<int, int> > confusionMatrix[2];

    Classifier(): posClass(0), bSingleClass(true), bUsesDrawTimer(true), bMultiClass(false)
	{
		rocdata.push_back(std::vector<f32pair>());
		rocdata.push_back(std::vector<f32pair>());
		roclabels.push_back("training");
		roclabels.push_back("testing");
	};

    virtual ~Classifier(){}
    std::vector <fvec> GetSamples(){return samples;}

    virtual void Train(std::vector< fvec > samples, ivec labels){}
    virtual fvec TestMulti(const fvec &sample){ return fvec(1,Test(sample));}
    virtual float Test(const fvec &sample){ return 0; }
    virtual float Test(const fVec &sample){ if(dim==2) return Test((fvec)sample); fvec s = (fvec)sample; s.resize(dim,0); return Test(s);}
    virtual const char *GetInfoString(){return NULL;}
    virtual void SaveModel(std::string filename){}
    virtual bool LoadModel(std::string filename){return false;}
    bool SingleClass(){return bSingleClass;}
    bool UsesDrawTimer(){return bUsesDrawTimer;}
    bool IsMultiClass(){return bMultiClass;}
    int Dim(){return dim;}
};

#endif // _CLASSIFIER_H_
