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
#ifndef _PROJECTOR_KPCA_H_
#define _PROJECTOR_KPCA_H_

#include <vector>
#include <projector.h>
#include "eigen_pca.h"

class ProjectorKPCA : public Projector
{
private:
	PCA *pca;
	fvec mean;
	fvec minValues, maxValues;
	ivec labels;
	int kernelType;
	int kernelDegree;
	float kernelGamma;
public:
	std::vector<fvec> Project(std::vector<fvec> samples);
        ivec GetLabels(){return labels;}

        ProjectorKPCA();
        void Train(std::vector< fvec > samples, ivec labels);
        fvec Project(const fvec &sample);

        char *GetInfoString();
        void SetParams(int kernelType, int kernelDegree, float kernelGamma);
};

#endif // _PROJECTOR_KPCA_H_
