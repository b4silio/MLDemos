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
#include "public.h"
#include "dynamicalASVM.h"
#include <QDebug>

using namespace std;


DynamicalASVM::DynamicalASVM()
    : classCount(0), nbClusters(2), kernelWidth(0.1), Cparam(1000), alphaTol(0.001), betaTol(0.001), betaRelax(0.0001), resizeFactor(10.f), maxIter(1e8), epsilon(0.01)
{
	endpoint = fvec();
	endpoint.resize(4,0.f);
}

DynamicalASVM::~DynamicalASVM()
{
}

void DynamicalASVM::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
{
	if(!trajectories.size()) return;
	int count = trajectories[0].size();
	if(!count) return;
    dim = trajectories[0][0].size();
	// we forget about time and just push in everything

    classMap.clear();
    int cnt=0;
    FOR(i, labels.size()) if(!classMap.count(labels[i])) classMap[labels[i]] = cnt++;
    for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++) inverseMap[it->second] = it->first;
    ivec newLabels(labels.size());
    FOR(i, labels.size()) newLabels[i] = classMap[labels[i]];

    int maxClass = 0;
    FOR(j, newLabels.size()) maxClass = max(maxClass, newLabels[j]);
    classCount = classMap.size();

    std::vector< std::vector< std::vector<fvec> > > trajectoryList(classCount);
    FOR(i, trajectories.size())
    {
        trajectoryList[newLabels[i]].push_back(trajectories[i]);
    }

    FOR(i, gmms.size()) DEL(gmms[i]);
    gmms.clear();

    FOR(c, classCount)
    {
        vector<fvec> samples;
        FOR(i, trajectoryList[c].size())
        {
            FOR(j, trajectoryList[c][i].size())
            {
                samples.push_back(trajectoryList[c][i][j]);
            }
        }
        if(!samples.size()) return;

        float *data = new float[samples.size()*dim];
        FOR(i, samples.size())
        {
            FOR(j, dim) data[i*dim + j] = samples[i][j];
        }

        // first learn the model with gmm
        Gmm *gmm = new Gmm(nbClusters, dim);
        gmm->init(data, samples.size(), 2); // kmeans initialization
        gmm->em(data, samples.size(), 1e-4, COVARIANCE_FULL);
        gmms.push_back(gmm);
        gmm->initRegression(dim/2);

        delete [] data;
    }

    dim = dim/2; // ASVM only takes X, not X_dot

    // we fill in the data for ASVM training
    asvmdata input;
    input.dim = dim;
    input.tar.resize(trajectoryList.size());

    FOR(i, trajectoryList.size())
    {
        input.tar[i].dim = dim;
        input.tar[i].targ = new double[dim];
        FOR(l, dim) input.tar[i].targ[l] = 0.0;

        input.tar[i].traj.resize(trajectoryList[i].size());
        FOR(j,trajectoryList[i].size())
        {
            unsigned int nPoints = trajectoryList[i][j].size();
            trajectory& traj = input.tar[i].traj[j];
            traj.nPoints = nPoints;
            traj.dim = dim;
            traj.coords = new double*[nPoints];
            traj.y = new int[nPoints];
            traj.vel = new double*[nPoints];

            FOR(k,nPoints)
            {
                traj.coords[k] = new double[dim];
                traj.vel[k] = new double[dim];

                FOR(l,dim)
                {
                    traj.coords[k][l] = trajectoryList[i][j][k][l];
                }
                traj.y[k] = i;
            }
            FOR(l, dim) input.tar[i].targ[l] +=  traj.coords[nPoints-1][l];
        }
        FOR(l, dim) input.tar[i].targ[l] /=  input.tar[i].traj.size();
    }
    double kernel_width = kernelWidth;
    double initial_guess = 1e-5;
    input.setParams("rbf", kernel_width, initial_guess);

    ASVM_SMO_Solver smo_solver;
    smo_solver.alpha_tol = alphaTol;
    smo_solver.beta_tol = betaTol;
    smo_solver.beta_relax = betaRelax;
    smo_solver.Cparam = Cparam;
    smo_solver.max_iter = maxIter;

    asvms.clear();
    asvms.resize(classCount);
    FOR(i, classCount)
    {
        smo_solver.learn(input, i, &asvms[i]);
    }
}

std::vector<fvec> DynamicalASVM::Test( const fvec &sample, int count)
{
    fvec start = sample;
    vector<fvec> res(count);
    FOR(i, count)
    {
        res[i] = start;
        fvec diff = Test(start);
        start += diff*dT;
    }
    return res;
}

fvec DynamicalASVM::Classify( const fvec &sample)
{
    int dim = sample.size();
    if(!asvms.size()) return fvec(2,0);
    if(!dim) return fvec(2,0);
    double *point = new double[dim];
    double *derivative = new double[dim];
    FOR(d, dim) point[d] = sample[d];
    double maxScore = -DBL_MAX;
    int maxIndex = 0;
    FOR(i, asvms.size())
    {
        double score = asvms[i].getclassifiervalue(point);
        if(maxScore < score)
        {
            maxScore = max(score, maxScore);
            maxIndex = i;
        }
    }
    //asvms[maxIndex].getclassifierderivative(point, derivative);

    fvec res(2);
    res[0] = maxIndex;
    res[1] = maxScore;
    delete [] point;
    delete [] derivative;
    return res;
}

fvec DynamicalASVM::Test( const fvec &sample)
{
	int dim = sample.size();
    if(!asvms.size()) return fvec(2,0);
	if(!dim) return fvec(2,0);
    double *point = new double[dim];
    double *derivative = new double[dim];
    FOR(d, dim) point[d] = sample[d];
    double maxScore = -DBL_MAX;
    int maxIndex = 0;
    FOR(i, asvms.size())
    {
        double score = asvms[i].getclassifiervalue(point);
        if(maxScore < score)
        {
            maxScore = max(score, maxScore);
            maxIndex = i;
        }
    }
    //fvec velocity(dim,0);
    float *sigma = new float[dim*(dim+1)/2];
    float *velocity = new float[dim];
    gmms[maxIndex]->doRegression(&sample[0], velocity, sigma);
    delete [] sigma;
    asvms[maxIndex].getclassifierderivative(point, derivative);
    fvec deriv(dim);
    FOR(d, dim) deriv[d] = derivative[d] / resizeFactor;
    float norm=sqrtf(deriv*deriv);
    FOR(d, dim) deriv[d] /= norm;

    fvec vel(dim);
    FOR(d, dim) vel[d] = velocity[d];
    // here we must combine the derivative from ASVM and the velocity from the DS
    float dot = vel*deriv;
    fvec residual = vel - deriv*dot;
    dot = max(epsilon,dot);
    vel = residual + deriv*dot;

    delete [] point;
    delete [] velocity;
    delete [] derivative;
    return vel;
}

fVec DynamicalASVM::Test( const fVec &sample)
{
    return Test(fvec(sample));
}

void DynamicalASVM::SetParams(int clusters, double kernelWidth, double Cparam, double alphaTol, double betaTol, double betaRelax, double epsilon, int maxIter)
{
    this->maxIter = maxIter;
    this->nbClusters = clusters;
    this->kernelWidth = kernelWidth;
    this->Cparam = Cparam;
    this->alphaTol = alphaTol;
    this->betaTol = betaTol;
    this->betaRelax = betaRelax;
    this->epsilon = epsilon;
}

const char *DynamicalASVM::GetInfoString()
{
	char *text = new char[2048];
    sprintf(text, "ASVM\n");
	sprintf(text, "%sMixture Components: %d\n", text, nbClusters);
    sprintf(text, "%sTraining Parameters: ", text);
    sprintf(text, "%sAlpha Tolerance: %f\n", text, alphaTol);
    sprintf(text, "%sBeta Tolerance: %f\n", text, betaTol);
    sprintf(text, "%sBeta Relaxation: %f\n", text, betaRelax);
    sprintf(text, "%sKernel Width: %f\n", text, kernelWidth);
    sprintf(text, "%sPenalty (C): %f\n\n", text, Cparam);
    FOR(i, asvms.size())
    {
        sprintf(text, "%sClass %d\n", text, i+1);
        sprintf(text, "%sAlpha Support Vectors: %d\n", text, asvms[i].numAlpha);
        sprintf(text, "%sBeta Support Vectors: %d\n", text, asvms[i].numBeta);
    }
    return text;
}

void DynamicalASVM::SaveModel(string filename)
{
}

bool DynamicalASVM::LoadModel(string filename)
{
    return true;
}
