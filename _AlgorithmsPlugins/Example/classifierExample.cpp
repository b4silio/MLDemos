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
#include "classifierExample.h"
#include <map>
#include <QDebug>

using namespace std;

void ClassifierExample::Train(std::vector< fvec > samples, ivec labels)
{
    // we don't want to train anything if we don't have data
	if(!samples.size()) return;

    // we determine the size of the data
    dim = samples[0].size();

    // we split the samples into separate lists for each class
    vector< fvec > positives, negatives;
	classes.clear();
    classMap.clear();
    inverseMap.clear();

    // we go through all the samples and either make a list of classes, or use the "positive vs negative" grouping
    int cnt=0;
    FOR(i, labels.size()) if(!classMap.count(labels[i])) classMap[labels[i]] = cnt++;
    for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++) inverseMap[it->second] = it->first;
    ivec newLabels(labels.size());
    FOR(i, labels.size()) newLabels[i] = classMap[labels[i]];

    for(map<int,int>::iterator it=inverseMap.begin(); it != inverseMap.end(); it++) qDebug() << "inverse: " << it->first << it->second;
    for(map<int,int>::iterator it=classMap.begin(); it != classMap.end(); it++) qDebug() << "class: " << it->first << it->second;

    std::map<int, vector<fvec> > sampleMap;

    FOR(i, samples.size())
	{
        sampleMap[newLabels[i]].push_back(samples[i]);
        if(newLabels[i] == 1) positives.push_back(samples[i]);
		else negatives.push_back(samples[i]);
	}

    // to give an example, we compute the center of each class
    centers.clear();

    // we iterate through the list of samples split by class
    for(map<int,vector<fvec> >::iterator it=sampleMap.begin(); it != sampleMap.end(); it++)
	{
        qDebug() << "analyzing class #" << it->first;

        // we get the list for the current class
        vector<fvec> &s = it->second;

        // we initialize the mean at zero
        fvec mean(dim, 0);
        // we compute the mean
        FOR(j, s.size())
        {
            mean += s[j];
        }
        mean /= s.size();

        // and we push it in the list by class
        centers[it->first] = mean;
	}
}

fvec ClassifierExample::TestMulti(const fvec &sample)
{
    fvec res(centers.size(),0);

    // we simply compute the distance from the center to the current sample
    for(map<int,fvec>::iterator it=centers.begin(); it != centers.end(); it++)
    {
        // we compute the difference
        fvec diff = sample - it->second;
        // we compute the dot product
        float x = sqrtf(diff*diff);
        // we use a simple rbf distance
        res[it->first] = expf(-0.5*x);
    }
    return res;
}

float ClassifierExample::Test(const fvec &sample)
{
    fvec likelihood = TestMulti(sample);
    if(likelihood.size() < 2) return 0;
    float res = log(likelihood[1]) - log(likelihood[0]);
	return res;
}

const char *ClassifierExample::GetInfoString()
{
    char *text = new char[1024];
    sprintf(text, "My Classifier Example\n");
    sprintf(text, "%s\n", text);
    sprintf(text, "%sTraining informations:\n", text);

    // here you can fill in whatever information you want
    for(map<int,fvec>::iterator it=centers.begin(); it != centers.end(); it++)
    {
        sprintf(text, "%sCenter for class %d\n", text, it->first);
        FOR(d, it->second.size())
        {
            // write down the dimension as floats with 3 decimals
            sprintf(text,"%s %.3f", text, it->second[d]);
        }
        sprintf(text, "%s\n", text);
    }

    return text;
}
