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
#include "algorithmmanager.h"

using namespace std;

void AlgorithmManager::QueryClassifier(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    fvec result;
    result.resize(1);
    if (classifier && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            fvec sample = samples[i];
            if (sourceDims.size()) {
                fvec newSample(sourceDims.size());
                FOR (d, sourceDims.size()) newSample[d] = sample[sourceDims[d]];
                sample = newSample;
            }
            result[0] = classifier->Test(sample);
            results[i] = result;
        }
    }
    emit SendResults(results);
}

void AlgorithmManager::QueryRegressor(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    if (regressor && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            results[i] = regressor->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void AlgorithmManager::QueryDynamical(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    if (dynamical && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            results[i] = dynamical->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void AlgorithmManager::QueryClusterer(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    if (clusterer && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            results[i] = clusterer->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void AlgorithmManager::QueryMaximizer(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    if (maximizer && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            results[i] = maximizer->Test(samples[i]);
        }
    }
    emit SendResults(results);
}

void AlgorithmManager::QueryProjector(std::vector<fvec> samples)
{
    std::vector<fvec> results;
    QMutexLocker lock(mutex);
    if (projector && samples.size()) {
        results.resize(samples.size());
        FOR (i, samples.size()) {
            results[i] = projector->Project(samples[i]);
        }
    }
    emit SendResults(results);
}
