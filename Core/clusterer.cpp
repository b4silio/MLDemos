#include <iostream>
#include "clusterer.h"

using std::vector;

fvec Clusterer::TestMany(const fvec &sampleMatrix, const int dim, const int count)
{
    fvec results;
    int resDim = -1;
    if(sampleMatrix.size() != dim*count) return results;
    FOR(i, count) {
        fvec sample(dim);
        FOR(d,dim) sample[d] = sampleMatrix[i*dim + d];
        fvec res = Test(sample);
        if(resDim == -1) {
            resDim = res.size();
            results.resize(count*resDim);
        }
        FOR(d, resDim) results[i*resDim + d] = res[d];
    }
    return results;
}

float Clusterer::GetLogLikelihood(std::vector<fvec> samples){
    if(!samples.size()) return 0;

    vector< vector<fvec> > samplesPerCluster(nbClusters);
    FOR(i, samples.size()) {
        fvec scores = Test(samples[i]);
        float maxScore = 0;
        int clusterIndex = 0;
        FOR(j, nbClusters) {
            if(scores[j] > maxScore) {
                maxScore = scores[j];
                clusterIndex = j;
            }
        }
        samplesPerCluster.at(clusterIndex).push_back(samples.at(i));
    }

    // compute the global and cluster-wise means of the data
    fvec means(dim,0);
    FOR ( i, samples.size() ) {
        FOR ( d, dim ) means[d] += samples[i][d];
    }
    FOR ( d, dim ) means[d] /= samples.size();

    vector<fvec> meansPerCluster(nbClusters);
    FOR (c, nbClusters) {
        vector<fvec>& samples = samplesPerCluster.at(c);
        fvec means(dim,0);
        FOR ( i, samples.size() ) {
            FOR ( d, dim ) means[d] += samples[i][d];
        }
        FOR ( d, dim ) means[d] /= samples.size();
        meansPerCluster[c] = means;
    }

    // compute the distribution variance
    fvec sigmas(dim);
    FOR ( i, samples.size() ) {
        FOR ( d, dim ) sigmas[d] += pow(samples[i][d] - means[d],2);
    }
    FOR ( d, dim ) sigmas[d] /= samples.size();

    vector<fvec> sigmasPerCluster(nbClusters);
    FOR (c, nbClusters) {
        vector<fvec>& samples = samplesPerCluster.at(c);
        fvec& means = meansPerCluster[c];
        fvec sigmas(dim);
        FOR ( i, samples.size() ) {
            FOR ( d, dim ) sigmas[d] += pow(samples[i][d] - means[d],2);
        }
        FOR ( d, dim ) sigmas[d] /= samples.size();
        sigmasPerCluster[c] = sigmas;
    }

    float loglik = 0;
    FOR(c, nbClusters) {
        int N = samplesPerCluster[c].size();
        if(N==0) continue;
        fvec& sigmas_c = sigmasPerCluster[c];
        float likelihood = 0;
        FOR(d, dim) {
            likelihood += 0.5*log(sigmas_c[d] + sigmas[d]);
        }
        likelihood *= -N;
        loglik += likelihood;
    }
    return loglik;
}
