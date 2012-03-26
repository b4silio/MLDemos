/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include <public.h>
#include "classifierMVM.h"
#include <QDebug>

using namespace std;

ClassifierMVM::ClassifierMVM()
{
    SVs = 0;
    alpha = 0;
    svCount = 0;
    b = 0;

    kernel_type = 0;
    degree = 1;
    gamma = 0.1;
    coef0 = 0.;
}

ClassifierMVM::~ClassifierMVM()
{
    if(SVs)
    {
        FOR(i, svCount)
        {
            KILL(SVs[i]);
        }
        KILL(SVs);
    }
    KILL(alpha);
}

void ClassifierMVM::SetParams(u32 kernelType, float kernelParam, ivec indices, fvec alphas)
{
    this->indices = indices;
    this->alphas = alphas;

    // default values
    coef0 = 0;
    gamma = 1;

    switch(kernelType)
    {
    case 0:
        kernel_type = LINEAR;
        degree = 1;
        break;
    case 1:
        kernel_type = POLY;
        degree = (u32)kernelParam;
        break;
    case 2:
        kernel_type = RBF;
        gamma = kernelParam;
        break;
    case 3:
        kernel_type = SIGMOID;
        gamma = kernelParam;
        break;
    }
}

float Kernel(const float* x, const float *sv, const int dim, const int kernelType, const int degree, const float gamma, const float coef0)
{
    float sum = 0.f;
    switch(kernelType)
    {
    case 0: // LINEAR
    {
        FOR(d, dim) sum += x[d]*sv[d];
    }
        break;
    case 1: // POLY
    {
        FOR(d, dim) sum += x[d]*sv[d];
        sum += coef0;
        sum = powf(sum, degree);
    }
        break;
    case 2: // RBF
    {
        FOR(d, dim)
        {
            float diff = x[d]-sv[d];
            sum += (diff)*(diff)*gamma;
        }
        sum = expf(-sum);
    }
        break;
    case 3: // SIGMOID
        break;
    }

    return sum;
}

void ClassifierMVM::Train(std::vector< fvec > _samples, ivec _labels)
{
    if(!manualSamples.size()) return;
    svCount = indices.size();
    if(SVs)
    {
        FOR(i, svCount)
        {
            KILL(SVs[i]);
        }
        KILL(SVs);
        KILL(alpha);
    }
    if(!indices.size()) return;

    dim = manualSamples[0].size();

    SVs = new float*[indices.size()];
    alpha = new float[indices.size()];
    FOR(i, indices.size())
    {
        SVs[i] = new float[dim];
        FOR(d, dim)
        {
            SVs[i][d] = manualSamples[indices[i]][d];
        }
        alpha[i] = alphas[i];
    }

    // we compute the b;
    b = 0;
    float sum = 0;
    FOR(i, svCount)
    {
        float y = 0;
        FOR(j, svCount)
        {
            y += alpha[j]*Kernel(SVs[i], SVs[j], dim, kernel_type, degree, gamma, coef0);
        }
        sum += (y - manualLabels[i]);
    }
    b = sum / svCount;
}

float ClassifierMVM::Test( const fvec &sample )
{
    if(!SVs || !svCount) return 0.f;

    float estimate = 0;

    // we compute the kernel
    FOR(i, svCount)
    {
        estimate += alpha[i]*Kernel(&sample[0], SVs[i], dim, kernel_type, degree, gamma, coef0);
    }

    return estimate - b;
}

const char *ClassifierMVM::GetInfoString()
{
    char *text = new char[1024];
    sprintf(text, "MVM\n");
    sprintf(text, "%sKernel: ", text);
    switch(kernel_type)
    {
    case LINEAR:
        sprintf(text, "%s linear\n", text);
        break;
    case POLY:
        sprintf(text, "%s polynomial (deg: %d bias: %.3f width: %f)\n", text, degree, coef0, gamma);
        break;
    case RBF:
        sprintf(text, "%s rbf (gamma: %f)\n", text, gamma);
        break;
    case SIGMOID:
        sprintf(text, "%s sigmoid (%f %f)\n", text, gamma, coef0);
        break;
    }
    sprintf(text, "%sSupport Vectors: %d\n", text, svCount);
    return text;
}
