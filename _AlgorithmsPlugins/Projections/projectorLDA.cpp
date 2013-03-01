#include "projectorLDA.h"
#include <mymaths.h>
#include <Eigen/Core>
#include <Eigen/Eigen>
#include <Eigen/LU>
#include <QDebug>

#include <map>

using namespace std;
using namespace Eigen;

ProjectorLDA::ProjectorLDA()
    : ldaType(1) // by default choose standard LDA
{}

void ProjectorLDA::Train(std::vector< fvec > samples, ivec labels)
{
    projected.clear();
    source.clear();
    if(!samples.size()) return;
    source = samples;
    projected = samples;
    int dim = samples[0].size();
    VectorXd W(dim);
    MatrixXd C(dim,dim), invC(dim,dim);

    mean = fvec(dim);
    w = fvec(dim);
    vector<fvec> class1Samples, class2Samples;
    int class1 = labels[0];
    FOR(i, samples.size())
    {
        if(labels[i] == class1) class1Samples.push_back(samples[i]);
        else class2Samples.push_back(samples[i]);
        mean += samples[i];
    }
    mean /= samples.size();
    if(!class2Samples.size() || !class1Samples.size()) return;
    fvec class1Mean(dim), class2Mean(dim);
    FOR(i, class1Samples.size()) class1Mean += class1Samples[i];
    FOR(i, class2Samples.size()) class2Mean += class2Samples[i];
    class1Mean /= class1Samples.size();
    class2Mean /= class2Samples.size();
    VectorXd class1M(dim), class2M(dim);
    FOR(d, dim)
    {
        class1M(d) = class1Mean[d];
        class2M(d) = class2Mean[d];
    }

    if(!ldaType)
    {
        w = class2Mean - class1Mean;
        float norm = w*w;
        w /= sqrtf(norm);
        projected = vector<fvec>(samples.size());
        FOR(i, samples.size())
        {
            float p = w*(samples[i]-mean);
            projected[i] = w*p + mean;
        }
        return;
    }

    if(ldaType==2)
    {
        MatrixXd class1C(dim,dim), class2C(dim,dim);
        FOR(x, dim)
        {
            FOR(y, dim)
            {
                class1C(x,y) = 0;
                class2C(x,y) = 0;
            }
        }
        FOR(i, class1Samples.size())
        {
            fvec d = class1Samples[i] - class1Mean;
            for(int x=0; x<dim; x++)
            {
                class1C(x,x) += d[x]*d[x];
                for(int y=0; y<x; y++)
                {
                    class1C(x,y) += d[x]*d[y];
                }
            }
        }
        FOR(i, class2Samples.size())
        {
            fvec d = class2Samples[i] - class2Mean;
            for(int x=0; x<dim; x++)
            {
                class2C(x,x) += d[x]*d[x];
                for(int y=0; y<x; y++)
                {
                    class2C(x,y) += d[x]*d[y];
                }
            }
        }
        for(int x=0; x<dim; x++)
        {
            class1C(x,x) /= class1Samples.size()-1;
            class2C(x,x) /= class2Samples.size()-1;
            for(int y=0; y<x; y++)
            {
                class1C(x,y) /= class1Samples.size()-1;;
                class2C(x,y) /= class2Samples.size()-1;;
                class1C(y,x) = class1C(x,y);
                class2C(y,x) = class2C(x,y);
            }
        }
        C = class1C + class2C;
        invC = C.inverse();
    }
    else
    {
        FOR(x, dim)
        {
            FOR(y, dim)
            {
                C(y,x) = 0;
            }
        }
        FOR(i, class1Samples.size())
        {
            fvec d = class1Samples[i] - class1Mean;
            for(int x=0; x<dim; x++)
            {
                C(x,x) += d[x]*d[x];
                for(int y=0; y<x; y++)
                {
                    C(x,y) += d[x]*d[y];
                }
            }
        }
        FOR(i, class2Samples.size())
        {
            fvec d = class2Samples[i] - class2Mean;
            for(int x=0; x<dim; x++)
            {
                C(x,x) += d[x]*d[x];
                for(int y=0; y<x; y++)
                {
                    C(x,y) += d[x]*d[y];
                }
            }
        }
        int count = class1Samples.size() + class2Samples.size() - 1;
        for(int x=0; x<dim; x++)
        {
            C(x,x) /= count;
            for(int y=0; y<x; y++)
            {
                C(x,y) /= count;
                C(y,x) = C(x,y);
            }
        }
        invC = C.inverse();
    }

    W = invC*(class2M - class1M);
    W = W.normalized();
    w.resize(dim);
    FOR(d, dim) w[d] = W(d);

    projected = vector<fvec>(samples.size());
    FOR(i, samples.size())
    {
        float p = w*(samples[i]-mean);
        projected[i] = w*p + mean;
    }
}

fvec ProjectorLDA::Project(const fvec &sample)
{
    float p = w*(sample-mean);
    return w*p + mean;
}

