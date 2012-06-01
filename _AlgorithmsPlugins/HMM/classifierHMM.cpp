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
#include "classifierHMM.h"
#include <QDebug>
#include <QTime>
#include <algorithm>
#include <sstream>

using namespace std;

ClassifierHMM::~ClassifierHMM()
{
    int N = learnedHMM.size();
    FOR(i, N)
    {
        DEL(learnedHMM[i]);
        DEL(pi[i]);
        DEL(b[i]);
        DEL(a[i]);
    }
}

fvec ClassifierHMM::Train(std::vector< std::vector<fvec> > trajectories, ivec labels)
{
    if(!trajectories.size()) return fvec(0);
    int count = trajectories.size();
    if(!count) return fvec(0);
    dim = trajectories[0][0].size();

    if(!learnedHMM.size())
    {
        int N = learnedHMM.size();
        FOR(i, N)
        {
            DEL(learnedHMM[i]);
            DEL(pi[i]);
            DEL(b[i]);
            DEL(a[i]);
        }
    }

    //int nbDimensions = dim/2; This is only true for the dynamic case, not for the classifier HMM
    int nbDimensions = dim;
    int nbStates = states;
    int nbSymbols = nbSym;
    int seed = QTime::currentTime().msec();
    bool isGaussian = 1 - this->obsType;
    int whichEMMethod = trainType;
    int whichInitialVector = initType;
    int whichTransitionMatrix = transType;

    FOR(i, trajectories.size())
    {
        FOR(j, trajectories[i].size())
        {
            FOR(d, dim)
            {
                float v = trajectories[i][j][d];
                v *= (float)nbSym;
                trajectories[i][j][d] = v;
            }
        }
    }

    this->trajectories = trajectories;

    int NClass = *(std::max_element(labels.begin(),labels.end()));

    fvec normalizedLogProb(NClass);

    // Split overall trajectories into "per class" trajectories
    FOR(i, NClass)
    {
        data.push_back(vector< vector<fvec> >());
    }

    FOR(i, trajectories.size())
    {
        data[(int)(labels[i]-1)].push_back(trajectories[i]);
    }

    FOR(i, NClass)
    {

        switch(whichTransitionMatrix)
        {
            case 0:
                a.push_back(new CPlainStateTrans(nbStates, seed));
                break;
            case 1:
                a.push_back(new CPlainStateTrans(nbStates, seed, 0));
                break;
            case 2:
                a.push_back(new CPlainStateTrans(nbStates, seed, 1));
                break;
            case 3:
                a.push_back(new CPlainStateTrans(nbStates, seed, 2));
                break;
        }

        CObs *obsType;
        obsType = new CVectorObs(nbDimensions);
        b.push_back(new CVectorObsProb(nbSymbols, nbStates, nbDimensions, isGaussian));

        switch(whichInitialVector)
        {
            case 0:
                pi.push_back(new CInitStateProb(nbStates));
                break;
            case 1:
                pi.push_back(new CInitStateProb(nbStates, 0));
                break;
            case 2:
                pi.push_back(new CInitStateProb(nbStates, 1));
                break;
        }

        learnedHMM.push_back(new CHMM(a[i], b[i], pi[i]));
        cout << "Initial Model\n";
        learnedHMM[i]->Print(cout);

        CObsSeq *obsSeq;
        obsSeq = new CObsSeq(obsType, data[i]);

        switch (whichEMMethod){
            case 0:
                learnedHMM[i]->LearnBaumWelch(obsSeq);
                break;
            case 1:
                learnedHMM[i]->LearnSegmentalKMeans(obsSeq);
                break;
            case 2:
                learnedHMM[i]->LearnHybridSKM_BW(obsSeq);
                break;
        }

        cout << "Learned Model\n";
        learnedHMM[i]->Print(cout);

        //cout << "States and Expected Observations\n";
        //learnedHMM->PrintStatesAndExpectedObs(obsSeq, cout, cout);

        // Distances to each sequence in obsSeq and total distance are stored in distanceOutput file
        //cout << "Viterbi Distances to each sequence\n";
        normalizedLogProb[i] = (float)learnedHMM[i]->FindDistance(obsSeq, cout);

        delete obsSeq;// check with purify
    }
    return normalizedLogProb;
}

bool ClassifierHMM::IsOrphanedState(int HMMClass, int state)
{
    if(state > states) return true;
    FOR(i, a[HMMClass]->GetN())
    {
        if(a[HMMClass]->at(state+1, i+1) != 1. / a[HMMClass]->GetN()) return false;
    }
    return true;
}

/*
fvec ClassifierHMM::Test( const std::vector< std::vector<fvec> > &trajectories)
{
    //int dim = sample.size();
    //if(!dim) return fvec(2,0);
    //fvec res(dim, 0);
    //return res;
    return fvec();
}
*/

fvec ClassifierHMM::Test( const std::vector<fvec> &trajectory)
{
    fvec logLik(learnedHMM.size());
    std::vector< std::vector<fvec> > SingletonTraj;
    SingletonTraj.resize(1);
    SingletonTraj[0] = trajectory;

    dim = trajectory[0].size();
    int nbDimensions = dim;

    // Multiply with the scalor nbSym
    FOR(j, SingletonTraj[0].size())
    {
        FOR(d, dim)
        {
            float v = SingletonTraj[0][j][d];
            v *= (float)nbSym;
            SingletonTraj[0][j][d] = v;
        }
    }


    CObs *obsType;
    obsType = new CVectorObs(nbDimensions);

    CObsSeq *obsSeq;
    obsSeq = new CObsSeq(obsType, SingletonTraj);

    for(int i=0;i<learnedHMM.size();i++)
    {
        logLik[i] = learnedHMM[i]->FindDistance(obsSeq, cout);
    }
    delete obsSeq, obsType;

    if(learnedHMM.size()==2){
        fvec res(1);
        res[0] =  logLik[1] - logLik[0];
        return res;
    }else{
        return logLik;
    }

}

void ClassifierHMM::SetParams(int nbSymbol, int states, int trainType, int obsType, int initType, int transType)
{
    this->nbSym = nbSymbol;
    this->states = states;
    this->trainType = trainType;
    this->obsType = obsType;
    this->initType = initType;
    this->transType = transType;
}

char *ClassifierHMM::GetInfoString()
{
    char *text = new char[2048];
    sprintf(text, "HMM\n");
    sprintf(text, "%sNumber of Symbols: %d\n", text, nbSym);
    sprintf(text, "%sStates: %d\n", text, states);
    sprintf(text, "%sTraining Method\n", text);
    switch(trainType)
    {
    case 0:
        sprintf(text, "%sBaum-Welch\n", text);
        break;
    case 1:
        sprintf(text, "%sSegmental K-Means\n", text);
        break;
    case 2:
        sprintf(text, "%sHybrid\n", text);
        break;
    }
    return text;
}

bool ClassifierHMM::LoadModel()
{
    std::ifstream hmmFile("HMM.txt",ifstream::in);

    if(hmmFile.is_open())
    {
        a.push_back(new CPlainStateTrans(hmmFile,this->states));
        if(this->obsType == 1)
        {
            b.push_back(new CVectorObsProb(hmmFile,this->states,1,0)); //Default dim = 1 for discrete
        }else{
            b.push_back(new CVectorObsProb(hmmFile,this->states,2,1)); //Default dim = 2 for continuous
        }
        pi.push_back(new CInitStateProb(hmmFile,this->states));
    }
    learnedHMM.push_back(new CHMM(a[a.size()-1], b[b.size()-1], pi[pi.size()-1]));
    cout << "Loaded HMM\n";
    learnedHMM[learnedHMM.size()-1]->Print(cout);
    hmmFile.close();
}

void ClassifierHMM::SaveModel()
{
    FOR(i,learnedHMM.size())
    {
        std::stringstream ss;
        ss<<(i+1);
        std::string filename = "HMM_"+ss.str()+".txt";
        std::ofstream newHMM(filename.c_str());
        if(newHMM.is_open()){
            learnedHMM[i]->Print(newHMM);
        }
        newHMM.close();

    }

}

void ClassifierHMM::GenerateData(std::vector<fvec>  &samples, ivec &labels, std::vector<ipair> &sequences, bool &projected)
{
    //Generate sequences from given HMMs

    // hardcoded for now, could be params for GUI in the future
    int nbSeqs = 100;
    int nbObs = 30;
    FOR(i, learnedHMM.size())
    {
        CObsSeq* seqs = learnedHMM[i]->GenerateSequences(nbSeqs,nbObs,QTime::currentTime().msec());

        // Build dataset from generated sequences
        for(int tr=1;tr<=nbSeqs;tr++)
        {
            sequences.push_back(ipair(samples.size(),0));
            for(int s=1;s<=nbObs;s++)
            {
                int nbDim = seqs->mObs[tr][s]->GetDim();
                fvec newSample(nbDim);
                for(int k=1;k<=nbDim;k++)
                {
                    newSample[k-1] = (float)(seqs->mObs[tr][s]->GetInt(k))/nbSym;

                }
                samples.push_back(newSample);
                labels.push_back(i+1);
            }
            sequences[sequences.size()-1].second = samples.size()-1;
        }
    }
    projected = false;
}
