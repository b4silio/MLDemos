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
#include "basicMath.h"
#include "mymaths.h"
#include "datasetManager.h"
#include <fstream>
#include <algorithm>
#include <map>

using namespace std;

/******************************************/
/*                                        */
/*    DATASET MANAGER                     */
/*                                        */
/******************************************/
u32 DatasetManager::IDCount = 0;

DatasetManager::DatasetManager(const int dimension)
: size(dimension)
{
    bProjected = false;
	ID = IDCount++;
	perm = NULL;
}

DatasetManager::~DatasetManager()
{
	Clear();
}

void DatasetManager::Clear()
{
    bProjected = false;
	samples.clear();
	obstacles.clear();
	flags.clear();
	labels.clear();
	sequences.clear();
	rewards.Clear();
    categorical.clear();
	KILL(perm);
}

void DatasetManager::AddSample(const fvec sample, const int label, const dsmFlags flag)
{
	if (!sample.size()) return;
    int dim = GetDimCount();
	size = sample.size();
    if(dim != size) // we need to go through all our data and adjust the dimensions
    {
        FOR(i, samples.size())
        {
            while(samples[i].size() < size) samples[i].push_back(0.f);
        }
    }
	samples.push_back(sample);
	labels.push_back(label);
	flags.push_back(flag);
	KILL(perm);
	perm = randPerm(samples.size());
}

void DatasetManager::AddSamples(const std::vector< fvec > newSamples, const ivec newLabels, const std::vector<dsmFlags> newFlags)
{
    if(!newSamples.size()) return;
    int dim = GetDimCount();
    size = newSamples[0].size();
    if(dim != size) // we need to go through all our data and adjust the dimensions
    {
        FOR(i, samples.size())
        {
            while(samples[i].size() < size) samples[i].push_back(0.f);
        }
    }
    FOR(i, newSamples.size())
	{
		if(newSamples[i].size())
		{
			samples.push_back(newSamples[i]);
			if(i < newFlags.size()) flags.push_back(newFlags[i]);
			else flags.push_back(_UNUSED);
		}
	}
	if(newLabels.size() == newSamples.size()) FOR(i, newLabels.size()) labels.push_back(newLabels[i]);
	else FOR(i, newSamples.size()) labels.push_back(0);
	KILL(perm);
	perm = randPerm(samples.size());
}

void DatasetManager::AddSamples(const DatasetManager &newSamples)
{
	AddSamples(newSamples.GetSamples(), newSamples.GetLabels(), newSamples.GetFlags());
}

void DatasetManager::RemoveSample(const unsigned int index)
{
	if(index >= samples.size()) return;
	if(samples.size() == 1)
	{
		Clear();
		return;
	}
	samples[index].clear();
	for (unsigned int i = index; i < samples.size()-1; i++)
	{
		samples[i] = samples[i+1];
		labels[i] = labels[i+1];
		flags[i] = flags[i+1];
	}
	samples.pop_back();
	labels.pop_back();
	flags.pop_back();

	// we need to check if a sequence needs to be shortened
	FOR(i, sequences.size())
	{
		if(sequences[i].first > index) // later sequences
		{
			sequences[i].first--;
			sequences[i].second--;
		}
		else if(sequences[i].first == index || sequences[i].second >= index)
		{
			sequences[i].second--;
		}
		if(sequences[i].first >= sequences[i].second) // we need to pop out the sequence
		{
			if(sequences[i].first == sequences[i].second)
			{
				flags[sequences[i].first] = _UNUSED;
			}
			for(int j=i; j<sequences.size()-1; j++)
			{
				sequences[j] = sequences[j+1];
			}
			sequences.pop_back();
			i--;
		}
	}
}

void DatasetManager::RemoveSamples(ivec indices)
{
    if(indices.size() > samples.size()) return;
    // we sort the indices
    sort(indices.begin(), indices.end(), less<int>());
    int offset = 0;
    FOR(i, indices.size())
    {
        int index = indices[i] - offset;
        if(index < 0 || index > samples.size()) continue;
        RemoveSample(index);
        offset++;
    }
}

void DatasetManager::AddSequence(const int start, const int stop)
{
	if(start >= samples.size() || stop >= samples.size()) return;
	for(int i=start; i<=stop; i++) flags[i] = _TRAJ;
	sequences.push_back(ipair(start,stop));
	// sort sequences by starting value
	std::sort(sequences.begin(), sequences.end());
}

void DatasetManager::AddSequence(const ipair newSequence)
{
	if(newSequence.first >= samples.size() || newSequence.second >= samples.size()) return;
	for(int i=newSequence.first; i<=newSequence.second; i++) flags[i] = _TRAJ;
	sequences.push_back(newSequence);
	// sort sequences by starting value
	std::sort(sequences.begin(), sequences.end());
}

void DatasetManager::AddSequences(const std::vector< ipair > newSequences)
{
	sequences.reserve(sequences.size()+newSequences.size());
	FOR(i, newSequences.size())
	{
		sequences.push_back(newSequences[i]);		
	}
}

void DatasetManager::RemoveSequence(const unsigned int index)
{
	if(index >= sequences.size()) return;
	for(int i=index; i<sequences.size()-1; i++) sequences[i] = sequences[i+1];
	sequences.pop_back();
}

void DatasetManager::AddTimeSerie(const std::string name, const std::vector<fvec> data, const std::vector<long int>  timestamps)
{
	TimeSerie serie;
	serie.name = name;
	serie.data = data;
	serie.timestamps = timestamps;
	AddTimeSerie(serie);
}

void DatasetManager::AddTimeSerie(const TimeSerie serie)
{
	series.push_back(serie);
}

void DatasetManager::AddTimeSeries(const vector<TimeSerie> newTimeSeries)
{
	series.insert(series.end(), newTimeSeries.begin(), newTimeSeries.end());
}

void DatasetManager::RemoveTimeSerie(const unsigned int index)
{
	if(index >= series.size()) return;
	series.erase(series.begin() + index);
}

void DatasetManager::AddObstacle(const fvec center, const fvec axes, const float angle, const fvec power, const fvec repulsion)
{
	Obstacle o;
	o.center = center;
	o.axes = axes;
	o.angle = angle;
	o.power = power;
	o.repulsion = repulsion;
	obstacles.push_back(o);
}

void DatasetManager::AddObstacles(const std::vector<Obstacle> newObstacles)
{
	FOR(i, newObstacles.size()) obstacles.push_back(newObstacles[i]);
}

void DatasetManager::RemoveObstacle(const unsigned int index)
{
	if(index >= obstacles.size()) return;
	for(int i=index; i<obstacles.size()-1; i++) obstacles[i] = obstacles[i+1];
	obstacles.pop_back();
}

void DatasetManager::AddReward(const float *values, const ivec size, const fvec lowerBoundary, const fvec higherBoundary)
{
	rewards.SetReward(values, size, lowerBoundary, higherBoundary);
}

// we compare the current sample with all the ones in the dataset
// and return the smallest distance
double DatasetManager::Compare(const fvec sample) const
{
	if(!sample.size()) return 1.0;

	// now compute the differences
	double minDist = 1.0;
	u32 index = 0;
	FOR(i, samples.size())
	{
		double dist = 0;
		FOR(j, size) dist += fabs(sample[j]-samples[i][j]);
		dist /= size;
		if(minDist > dist)
		{
			index = i;
			minDist = dist;
		}
	}
	return minDist;
}

void DatasetManager::Randomize(const int seed)
{
	KILL(perm);
	if(samples.size()) perm = randPerm(samples.size(), seed);
}

void DatasetManager::ResetFlags()
{
	FOR(i, samples.size()) flags[i] = _UNUSED;
}

void DatasetManager::SetSample(const int index, const fvec sample)
{
    if(index >= 0 && index < samples.size()) samples[index] = sample;
}

string DatasetManager::GetCategorical(const int dimension, const int value) const
{
    string s;
    if(categorical.count(dimension) && value < categorical.at(dimension).size())
    {
        s = categorical.at(dimension)[value];
    }
    return s;
}

bool DatasetManager::IsCategorical(const int dimension) const
{
    return categorical.count(dimension) > 0;
}

fvec DatasetManager::GetSampleDim(const int index, const ivec inputDims, const int outputDim) const
{
    if(index >= samples.size()) return fvec();
    if(!inputDims.size()) return samples[index];
    if(outputDim == -1)
    {
        fvec sample(inputDims.size());
        FOR(d, inputDims.size()) sample[d] = samples[index][inputDims[d]];
        return sample;
    }
    else
    {
        int outputIndex = -1;
        FOR(d, inputDims.size())
        {
            if(outputDim == inputDims[d])
            {
                outputIndex = d;
                break;
            }
        }
        fvec sample(inputDims.size() + (outputIndex==-1 ? 0 : 1));
        FOR(d, inputDims.size())
        {
            if(d==outputIndex) sample.back() = samples[index][inputDims[d]];
            else sample[d<outputIndex ? d : d-1] = samples[index][inputDims[d]];
        }
        if(outputIndex == -1) sample.back() = samples[index][outputDim];
        return sample;
    }

    int dim = inputDims.size();
    fvec sample(dim + outputDim!=-1?1:0);
    FOR(d, dim) sample[d] = samples[index][inputDims[d]];
    if(outputDim != -1) sample[dim] = samples[index][outputDim];
    return sample;
}

std::vector< fvec > DatasetManager::GetSampleDims(const ivec inputDims, const int outputDim) const
{
    if ( !inputDims.size() ) return samples;

    vector<fvec> newSamples = samples;
    int newDim = inputDims.size();
    if ( outputDim == -1 ) {
        FOR ( i, samples.size() ) {
            fvec newSample(newDim);
            FOR ( d, inputDims.size() ) {
                newSample[d] = samples[i][inputDims[d]];
            }
            newSamples[i] = newSample;
        }
    } else {
        int outputIndex = -1;
        FOR ( d, inputDims.size() ) {
            if ( outputDim == inputDims[d] ) {
                newDim--;
                break;
            }
        }
        FOR ( i, samples.size() ) {
            fvec newSample(newDim);
            if ( outputIndex == -1 ) {
                FOR ( d, newDim-1 ) {
                    newSample[d] = samples[i][inputDims[d]];
                }
                newSample[newDim-1] = samples[i][outputDim];
            } else {
                FOR ( d, newDim ) {
                    if ( d == outputIndex ) newSample[newDim-1] = samples[i][inputDims[d]];
                    else newSample[d<outputIndex?d:d-1] = samples[i][inputDims[d]];
                }
            }
            newSamples[i] = newSample;
        }
    }
    return newSamples;
}

std::vector< fvec > DatasetManager::GetSampleDims(const std::vector<fvec> samples, const ivec inputDims, const int outputDim) const
{
    if(!inputDims.size()) return samples;

    vector<fvec> newSamples = samples;
    int newDim = inputDims.size();
    if ( outputDim == -1 ) {
        FOR ( i, samples.size() ) {
            fvec newSample(newDim);
            FOR ( d, inputDims.size() ) {
                newSample[d] = samples[i][inputDims[d]];
            }
            newSamples[i] = newSample;
        }
    } else {
        int outputIndex = -1;
        FOR ( d, inputDims.size() ) {
            if ( outputDim == inputDims[d] ) {
                newDim--;
                break;
            }
        }
        FOR ( i, samples.size() ) {
            fvec newSample(newDim);
            if ( outputIndex == -1 ) {
                FOR ( d, newDim-1 ) {
                    newSample[d] = samples[i][inputDims[d]];
                }
                newSample[newDim-1] = samples[i][outputDim];
            } else {
                FOR ( d, newDim ) {
                    if ( d == outputIndex ) newSample[newDim-1] = samples[i][inputDims[d]];
                    else newSample[d<outputIndex?d:d-1] = samples[i][inputDims[d]];
                }
            }
            newSamples[i] = newSample;
        }
    }
    return newSamples;
}

std::vector< fvec > DatasetManager::GetSamples(const u32 count, const dsmFlags flag, const dsmFlags replaceWith)
{
	std::vector< fvec > selected;
	if (!samples.size() || !perm) return selected;

	if (!count)
	{
		FOR(i, samples.size())
		{
			if ( flags[perm[i]] == flag)
			{
				selected.push_back(samples[perm[i]]);
				flags[perm[i]] = replaceWith;
			}
		}
		return selected;
	}

	for ( u32 i=0, cnt=0; i < samples.size() && cnt < count; i++ )
	{
		if ( flags[perm[i]] == flag )
		{
			selected.push_back(samples[perm[i]]);
			flags[perm[i]] = replaceWith;
			cnt++;
		}
	}

	return selected;
}

std::vector< std::vector < fvec > > DatasetManager::GetTrajectories(const int resampleType, const int resampleCount_, const int centerType, const float dT, const int zeroEnding) const
{
    int resampleCount = resampleCount_;
	// we split the data into trajectories
	vector< vector<fvec> > trajectories;
	if(!sequences.size() || !samples.size()) return trajectories;
	int dim = samples[0].size();
	trajectories.resize(sequences.size());
	FOR(i, sequences.size())
	{
		int length = sequences[i].second-sequences[i].first+1;
		trajectories[i].resize(length);
		FOR(j, length)
		{
			trajectories[i][j].resize(dim*2);
			// copy data
			FOR(d, dim) trajectories[i][j][d] = samples[sequences[i].first + j][d];
		}
	}

	switch(resampleType)
	{
	case 0: // none
	{
		FOR(i,sequences.size())
		{
			int cnt = sequences[i].second-sequences[i].first+1;
			if(resampleCount > cnt) resampleCount = cnt;
		}
		FOR(i, trajectories.size())
		{
			while(trajectories[i].size() > resampleCount) trajectories[i].pop_back();
		}
	}
		break;
	case 1: // uniform
	{
		FOR(i, trajectories.size())
		{
			vector<fvec> trajectory = trajectories[i];
			trajectories[i] = interpolate(trajectory, resampleCount);
		}
	}
		break;
	case 2: // spline
	{
		FOR(i, trajectories.size())
		{
			vector<fvec> trajectory = trajectories[i];
			trajectories[i] = interpolateSpline(trajectory, resampleCount);
		}
	}
		break;
	}


	if(centerType)
	{
		map<int,int> counts;
		map<int,fvec> centers;
		vector<int> trajLabels(sequences.size());
		FOR(i, sequences.size())
		{
			int index = centerType==1 ? sequences[i].second : sequences[i].first; // start
			int label = GetLabel(index);
			trajLabels[i] = label;
			if(!centers.count(label))
			{
				fvec center(dim,0);
				centers[label] = center;
				counts[label] = 0;
			}
			centers[label] += samples[index];
			counts[label]++;
		}
		for(map<int,int>::iterator p = counts.begin(); p!=counts.end(); ++p)
		{
			int label = p->first;
			centers[label] /= p->second;
		}
		FOR(i, trajectories.size())
		{
			if(centerType == 1)
			{
				fvec difference = centers[trajLabels[i]] - trajectories[i].back();
				FOR(j, resampleCount) trajectories[i][j] += difference;
			}
			else
			{
				fvec difference = centers[trajLabels[i]] - trajectories[i][0];
				FOR(j, resampleCount) trajectories[i][j] += difference;
			}
		}
	}

	float maxV = -FLT_MAX;
	// we compute the velocity
	FOR(i, trajectories.size())
	{
		FOR(j, resampleCount-1)
		{
			FOR(d, dim)
			{
				float velocity = (trajectories[i][j+1][d] - trajectories[i][j][d]) / dT;
				trajectories[i][j][dim + d] = velocity;
				if(velocity > maxV) maxV = velocity;
			}
		}
		if(!zeroEnding)
		{
			FOR(d, dim)
			{
				trajectories[i][resampleCount-1][dim + d] = trajectories[i][resampleCount-2][dim + d];
			}
		}
	}

	// we normalize the velocities as the variance of the data
	fvec mean, sigma;
	mean.resize(dim,0);
	int cnt = 0;
	sigma.resize(dim,0);
	FOR(i, trajectories.size())
	{
		FOR(j, resampleCount)
		{
			mean += trajectories[i][j];
			cnt++;
		}
	}
	mean /= cnt;
	FOR(i, trajectories.size())
	{
		FOR(j, resampleCount)
		{
			fvec diff = (mean - trajectories[i][j]);
			FOR(d,dim) sigma[d] += diff[d]*diff[d];
		}
	}
	sigma /= cnt;

	FOR(i, trajectories.size())
	{
		FOR(j, resampleCount)
		{
			FOR(d, dim)
			{
				trajectories[i][j][dim + d] /= maxV;
				//trajectories[i][j][dim + d] /= sqrt(sigma[d]);
			}
		}
	}
	return trajectories;
}


void DatasetManager::Save(const char *filename)
{
    if(!samples.size() && rewards.Empty()) return;
	u32 sampleCnt = samples.size();
    if(sampleCnt) size = samples[0].size();

	ofstream file(filename);
	if(!file.is_open()) return;

	file << sampleCnt << " " << size << "\n";
	FOR(i, sampleCnt)
	{
		FOR(j,size)
		{
			file << samples[i][j] << " ";
		}
		file << labels[i] << " ";
		file << flags[i] << " ";
		file << "\n";
	}

	if(sequences.size())
	{
		file << "s " << sequences.size() << "\n";
		FOR(i, sequences.size())
		{
			file << sequences[i].first << " " << sequences[i].second << "\n";
		}
	}

	// we load the obstacles
    if(obstacles.size())
	{
		file << "o " << obstacles.size() << "\n";
		FOR(i, obstacles.size())
		{
			FOR(j, size) file << obstacles[i].center[j] << " ";
			FOR(j, size) file << obstacles[i].axes[j] << " ";
			file << obstacles[i].angle << " ";
			file << obstacles[i].power[0] << " ";
			file << obstacles[i].power[1] << " ";
			file << obstacles[i].repulsion[0] << " ";
			file << obstacles[i].repulsion[1] << "\n";
		}
	}
    if(!rewards.Empty())
    {
        file << "r " << rewards.dim << " " << rewards.length << "\n";
        FOR(i, rewards.dim)
        {
            file << rewards.size[i] << " " << rewards.lowerBoundary[i] << " " << rewards.higherBoundary[i] << "\n";
        }
        FOR(i, rewards.length)
        {
            file << rewards.rewards[i] << " ";
        }
    }

	file.close();
}

bool DatasetManager::Load(const char *filename)
{
	ifstream file(filename);
	if(!file.is_open()) return false;
	Clear();

	int sampleCnt;
	file >> sampleCnt;
	file >> size;

	// we load the samples
	FOR(i, sampleCnt)
	{
		fvec sample;
		sample.resize(size,0);
		int label, flag;
		FOR(j, size)
		{
			file >> sample[j];
		}
		file >> label;
		file >> flag;
		samples.push_back(sample);
		labels.push_back(label);
		flags.push_back((dsmFlags)flag);
	}

	// we load the sequences
	char tmp[255];
	file.getline(tmp,255); // we skip the rest of the line
	int nextChar = file.peek();
	if(nextChar == 's') // we have sequences!
	{
		char dump;
		file >> dump;
		int sequenceCount;
		file >> sequenceCount;
		FOR(i, sequenceCount)
		{
			int start, stop;
			file >> start;
			file >> stop;
			sequences.push_back(ipair(start,stop));
		}
		file.getline(tmp,255);
		nextChar = file.peek();
	}
	// we load the obstacles
	if(nextChar == 'o')
	{
		char dump;
		file >> dump;
		int obstacleCount;
		file >> obstacleCount;
		Obstacle obstacle;
		obstacle.center.resize(size);
		obstacle.axes.resize(size);
		obstacle.power.resize(size);
		obstacle.repulsion.resize(size);
		FOR(i, obstacleCount)
		{
			FOR(j, size) file >> obstacle.center[j];
			FOR(j, size) file >> obstacle.axes[j];
			file >> obstacle.angle;
			FOR(j, size) file >> obstacle.power[j];
			FOR(j, size) file >> obstacle.repulsion[j];
			obstacles.push_back(obstacle);
		}
	}
    // we load the reward
    if(nextChar == 'r')
    {
        char dump;
        file >> dump;
        int dims, length;
        file >> dims >> length;
        ivec size(dims);
        fvec lowerBoundary(dims), higherBoundary(dims);
        int testLength = 1;
        FOR(i, dims)
        {
            file >> size[i] >> lowerBoundary[i] >> higherBoundary[i];
            testLength *= size[i];
        }
        if(testLength == length)
        {
            double *rewardData = new double[length];
            FOR(i, length)
            {
                double value;
                file >> value;
                rewardData[i] = value;
            }
            rewards.lowerBoundary = lowerBoundary;
            rewards.higherBoundary = higherBoundary;
            rewards.size = size;
            rewards.dim = dims;
            rewards.length = length;
            if(rewards.rewards) delete [] rewards.rewards;
            rewards.rewards = rewardData;
        }
    }

	file.close();
	KILL(perm);
	perm = randPerm(samples.size());
	return samples.size() > 0;
}

int DatasetManager::GetDimCount() const
{
	int dim = 2;
    if(samples.size()) dim = samples.at(0).size();
    if(series.size() && series.at(0).size())
	{
        dim = series.at(0).at(0).size()+1;
	}
	return dim;
}

std::pair<fvec, fvec> DatasetManager::GetBounds() const
{
    if(!samples.size()) return make_pair(fvec(),fvec());
    int dim = samples[0].size();
    fvec mins(dim,FLT_MAX), maxes(dim,-FLT_MAX);
    FOR(i, samples.size())
    {
        const fvec& sample = samples.at(i);
        int dim = sample.size();
        FOR(d,dim)
        {
            if(mins[d] > sample[d]) mins[d] = sample.at(d);
            if(maxes[d] < sample[d]) maxes[d] = sample.at(d);
        }
    }
    return make_pair(mins, maxes);
}

u32 DatasetManager::GetClassCount(const ivec classes)
{
    int cnt=0;
    map<int,int> classMap;
    FOR(i, classes.size()) if(!classMap.count(classes[i])) classMap[classes[i]] = cnt++;
    return classMap.size();
}

bvec DatasetManager::GetFreeFlags() const
{
	bvec res;
	FOR(i, flags.size()) res.push_back(flags[i] == _UNUSED);
	return res;
}


/******************************************/
/*                                        */
/*    REWARD MAPS                         */
/*                                        */
/******************************************/
RewardMap& RewardMap::operator= (const RewardMap& r)
{
  if (this != &r) {
	  dim = r.dim;
	  size = r.size;
	  lowerBoundary = r.lowerBoundary;
	  higherBoundary = r.higherBoundary;
      if(length != r.length)
      {
          length = r.length;
          if(rewards) delete [] rewards;
          rewards = new double[length];
      }
      memcpy(rewards, r.rewards, length*sizeof(double));
  }
  return *this;
}

void RewardMap::SetReward(const double *rewards, const ivec size, const fvec lowerBoundary, const fvec higherBoundary)
{
	this->lowerBoundary = lowerBoundary;
	this->higherBoundary = higherBoundary;
	this->size = size;
	dim = size.size();
	length = 1;
	FOR(i, size.size()) length *= size[i];
	if(this->rewards) delete [] this->rewards;
    this->rewards = new double[length];
    memcpy(this->rewards, rewards, length*sizeof(double));
}

void RewardMap::SetReward(const float *rewards, const ivec size, const fvec lowerBoundary, const fvec higherBoundary)
{
    this->lowerBoundary = lowerBoundary;
    this->higherBoundary = higherBoundary;
    this->size = size;
    dim = size.size();
    length = 1;
    FOR(i, size.size()) length *= size[i];
    if(this->rewards) delete [] this->rewards;
    this->rewards = new double[length];
    FOR(i, length)
    {
        this->rewards[i] = (double)rewards[i];
    }
}

float *RewardMap::GetRewardFloat() const
{
    if(!length) return 0;
    float *rewards = new float[length];
    FOR(i, length) rewards[i] = this->rewards[i];
    return rewards;
}

void RewardMap::Clear()
{
	dim = 0;
	size.clear();
	length = 0;
	lowerBoundary.clear();
	higherBoundary.clear();
    KILL(rewards);
}

void RewardMap::Zero()
{
	FOR(i, length) rewards[i] = 0;
}

// return the value of the reward function at the coordinates provided
float RewardMap::ValueAt(fvec sample) const
{
	if(!rewards) return 0.f;
	ivec index;
	index.resize(dim);
	FOR(d, dim)
	{
		//we check if we're outside the boundaries
		if(sample[d] < lowerBoundary[d]) sample[d] = lowerBoundary[d];
		if(sample[d] > higherBoundary[d]) sample[d] = higherBoundary[d];
		// now we get the closest index on the map
		index[d] = (int)((sample[d] - lowerBoundary[d]) / (higherBoundary[d] - lowerBoundary[d]) * size[d]);
	}

	// we convert the map index to a vector index
	int rewardIndex = 0;
	FOR(d,dim)
	{
		rewardIndex = rewardIndex*size[dim-d-1]+index[dim-d-1];
	}

	//printf("sample: %f %f index: %d %d (%d) value: %f\n", sample[0], sample[1], index[0], index[1], rewardIndex, rewards[rewardIndex]);
	// TODO: return interpolation of closest indices instead of the closest index itself
	return rewards[rewardIndex];
}

void RewardMap::SetValueAt(const fvec sample, const double value)
{
	if(!rewards) return;
	ivec index;
	index.resize(dim);
	FOR(d, dim)
	{
		//we check if we're outside the boundaries
		if(sample[d] < lowerBoundary[d]) return;
		if(sample[d] > higherBoundary[d]) return;
		// now we get the closest index on the map
		index[d] = (int)((sample[d] - lowerBoundary[d]) / (higherBoundary[d] - lowerBoundary[d]) * size[d]);
	}

	// we convert the map index to a vector index
	int rewardIndex = 0;
	FOR(d,dim)
	{
		rewardIndex = rewardIndex*size[dim-d-1]+index[dim-d-1];
	}
	rewards[rewardIndex] = value;
}

void RewardMap::ShiftValueAt(const fvec sample, const double shift)
{
	if(!rewards) return;
	ivec index;
	index.resize(dim);
	FOR(d, dim)
	{
		//we check if we're outside the boundaries
		if(sample[d] < lowerBoundary[d]) return;
		if(sample[d] > higherBoundary[d]) return;
		// now we get the closest index on the map
		index[d] = (int)((sample[d] - lowerBoundary[d]) / (higherBoundary[d] - lowerBoundary[d]) * size[d]);
	}
	// we convert the map index to a vector index
	int rewardIndex = 0;
	FOR(d,dim)
	{
		rewardIndex = rewardIndex*size[dim-d-1]+index[dim-d-1];
	}
	printf("index: %d value: %f\n", rewardIndex, rewards[rewardIndex]);
	rewards[rewardIndex] += shift;
}

void RewardMap::ShiftValueAt(const fvec sample, const double radius, const double shift)
{
	if(!rewards) return;
	ivec index;
	index.resize(dim);
	ivec lowIndex = index, hiIndex = index;
	ivec steps; steps.resize(dim);
	FOR(d, dim)
	{
		//we check if we're outside the boundaries
		if(sample[d] < lowerBoundary[d]) return;
		if(sample[d] > higherBoundary[d]) return;
		// now we get the closest index on the map
		steps[d] = (int)(2*radius / (higherBoundary[d] - lowerBoundary[d]) * size[d]);
		index[d] = (int)((sample[d] - lowerBoundary[d]) / (higherBoundary[d] - lowerBoundary[d]) * size[d]);
		lowIndex[d] = (int)((sample[d] - radius - lowerBoundary[d]) / (higherBoundary[d] - lowerBoundary[d]) * size[d]);
	}
	FOR(i, steps[1])
	{
		FOR(j, steps[0])
		{
			float x = 2.f*(j - steps[0]*0.5f)/float(steps[0]);
			float y = 2.f*(i - steps[1]*0.5f)/float(steps[0]);
			if(x*x + y*y > 1) continue;
			// we convert the map index to a vector index
			int rewardIndex = index[0] - steps[0]/2 + j + (index[1] - steps[1]/2 + i)*size[0];
			if(rewardIndex < 0 || rewardIndex>=length) return;
			rewards[rewardIndex] += shift;
		}
	}
}
