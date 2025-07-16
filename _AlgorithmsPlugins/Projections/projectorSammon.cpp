#include "projectorSammon.h"
#include <mymaths.h>
#include <QDebug>

#include <map>

using namespace std;
using namespace dlib;

ProjectorSammon::ProjectorSammon()
{}

void ProjectorSammon::Train(std::vector< fvec > samples, ivec labels)
{
    projected.clear();
    source.clear();
    if(!samples.size()) return;
    source = samples;
    projected = samples;
    dim = samples[0].size();

    std::vector< matrix<double,0,1> > data(samples.size());
    FOR(i, samples.size())
    {
        data[i].set_size(dim);
        FOR(d, dim) data[i](d) = samples[i][d];
    }

    sammon_projection projection;
    std::vector< matrix<double,0,1> > proj = projection(data, num_dims);
    projected.resize(proj.size());
    FOR(i, proj.size())
    {
        projected[i].resize(num_dims);
        FOR(d, num_dims) projected[i][d] = proj[i](d);
    }
}

fvec ProjectorSammon::Project(const fvec &sample)
{
    // this is a lame solution for reprojecting stuff
    // we look for the closest point in our sources and return the corresponding projection
    int closest = 0;
    float minDist = FLT_MAX;
    FOR(i, source.size())
    {
        float dist = (source[i]-sample)*(source[i]-sample);
        if(dist < minDist)
        {
            minDist = dist;
            closest = i;
        }
    }
    if(closest >= projected.size()) return fvec (dim, 0);
    else return projected[closest];
}

