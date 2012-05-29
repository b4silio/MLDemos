#ifndef CLASSIFIERHMM_H
#define CLASSIFIERHMM_H

// LAMP_HMM code from
// http://www.cfar.umd.edu/~daniel/Site_2/Code.html
#include <vector>
#include "classifier.h"
#include <LAMP_HMM/utils.h>
#include <LAMP_HMM/obs.h>
#include <LAMP_HMM/obsSeq.h>
#include <LAMP_HMM/obsProb.h>
#include <LAMP_HMM/discreteObsProb.h>
#include <LAMP_HMM/gaussianObsProb.h>
#include <LAMP_HMM/vectorObsProb.h>
#include <LAMP_HMM/stateTrans.h>
#include <LAMP_HMM/plainStateTrans.h>
#include <LAMP_HMM/gammaProb.h>
#include <LAMP_HMM/explicitDurationTrans.h>
#include <LAMP_HMM/initStateProb.h>
#include <LAMP_HMM/hmm.h>

class ClassifierHMM : public Classifier
{
public:
    int states, trainType, obsType, initType, transType, nbSym;
    std::vector<CStateTrans*> a;
    std::vector<CObsProb*> b;
    std::vector<CInitStateProb*> pi;
    std::vector<CHMM*> learnedHMM;
    std::vector< std::vector<fvec> > trajectories;
    std::vector< std::vector<std::vector<fvec> > > data;
public:
    ClassifierHMM() : nbSym(1), states(1), trainType(1), obsType(0), initType(0), transType(0), a(0), b(0), pi(0), learnedHMM(0){};
    ~ClassifierHMM();
    fvec Train(std::vector< std::vector<fvec> > trajectories, ivec labels);
    fvec Test(const std::vector<fvec> &trajectory);
    char *GetInfoString();
    bool IsOrphanedState(int HMMClass, int state);
    bool handlesTrajectories(){return true;}
    void SetParams(int mixtures, int states, int trainType, int obsType, int initType, int transType);
    bool LoadModel();
    void SaveModel();
    void GenerateData(std::vector<fvec> &samples, ivec &labels, std::vector<ipair> &sequences, bool &projected);
};

#endif // CLASSIFIERHMM_H
