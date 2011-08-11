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
#include <map>

using namespace std;

u32 DatasetManager::IDCount = 0;

DatasetManager::DatasetManager(int dimension)
: size(dimension)
{
	ID = IDCount++;
	perm = NULL;
}

DatasetManager::~DatasetManager()
{
	Clear();
}

void DatasetManager::Clear()
{
	samples.clear();
	obstacles.clear();
	flags.clear();
	labels.clear();
	sequences.clear();
	rewards.Clear();
	KILL(perm);
}

void DatasetManager::AddSample(fvec sample, int label, dsmFlags flag)
{
	if (!sample.size()) return;
	size = sample.size();

	samples.push_back(sample);
	labels.push_back(label);
	flags.push_back(flag);
	KILL(perm);
	perm = randPerm(samples.size());
}

void DatasetManager::AddSamples(std::vector< fvec > newSamples, ivec newLabels, std::vector<dsmFlags> newFlags)
{
	FOR(i, newSamples.size())
	{
		if(newSamples[0].size()) size = newSamples[0].size();
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

void DatasetManager::AddSamples(DatasetManager &newSamples)
{
	AddSamples(newSamples.GetSamples(), newSamples.GetLabels(), newSamples.GetFlags());
}

void DatasetManager::RemoveSample(unsigned int index)
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

void DatasetManager::AddSequence(int start, int stop)
{
	if(start >= samples.size() || stop >= samples.size()) return;
	for(int i=start; i<=stop; i++) flags[i] = _TRAJ;
	sequences.push_back(ipair(start,stop));
	// sort sequences by starting value
	std::sort(sequences.begin(), sequences.end());
}

void DatasetManager::AddSequence(ipair newSequence)
{
	if(newSequence.first >= samples.size() || newSequence.second >= samples.size()) return;
	for(int i=newSequence.first; i<=newSequence.second; i++) flags[i] = _TRAJ;
	sequences.push_back(newSequence);
	// sort sequences by starting value
	std::sort(sequences.begin(), sequences.end());
}

void DatasetManager::AddSequences(std::vector< ipair > newSequences)
{
	sequences.reserve(sequences.size()+newSequences.size());
	FOR(i, newSequences.size())
	{
		sequences.push_back(newSequences[i]);		
	}
}

void DatasetManager::RemoveSequence(unsigned int index)
{
	if(index >= sequences.size()) return;
	for(int i=index; i<sequences.size()-1; i++) sequences[i] = sequences[i+1];
	sequences.pop_back();
}

void DatasetManager::AddObstacle(fvec center, fvec axes, float angle, fvec power, fvec repulsion)
{
	Obstacle o;
	o.center = center;
	o.axes = axes;
	o.angle = angle;
	o.power = power;
	o.repulsion = repulsion;
	obstacles.push_back(o);
}

void DatasetManager::AddObstacles(std::vector<Obstacle> newObstacles)
{
	FOR(i, newObstacles.size()) obstacles.push_back(newObstacles[i]);
}

void DatasetManager::RemoveObstacle(unsigned int index)
{
	if(index >= obstacles.size()) return;
	for(int i=index; i<obstacles.size()-1; i++) obstacles[i] = obstacles[i+1];
	obstacles.pop_back();
}

void DatasetManager::AddReward(float *values, ivec size, fvec lowerBoundary, fvec higherBoundary)
{
	rewards.SetReward(values, size, lowerBoundary, higherBoundary);
}

// we compare the current sample with all the ones in the dataset
// and return the smallest distance
double DatasetManager::Compare(fvec sample)
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

void DatasetManager::Randomize(int seed)
{
	KILL(perm);
	if(samples.size()) perm = randPerm(samples.size(), seed);
}

void DatasetManager::ResetFlags()
{
	FOR(i, samples.size()) flags[i] = _UNUSED;
}

void DatasetManager::SetSample(int index, fvec sample)
{
	if(index >= 0 && index < samples.size()) samples[index] = sample;
}

std::vector< fvec > DatasetManager::GetSamples(u32 count, dsmFlags flag, dsmFlags replaceWith)
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

std::vector< std::vector < fvec > > DatasetManager::GetTrajectories(int resampleType, int resampleCount, int centerType, float dT, int zeroEnding)
{

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
	if(!samples.size()) return;
	u32 sampleCnt = samples.size();

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

	file.close();
	KILL(perm);
	perm = randPerm(samples.size());
	return samples.size() > 0;
}

u32 DatasetManager::GetClassCount(ivec classes)
{
	u32 counts[256];
	memset(counts, 0, 256*sizeof(u32));
	FOR(i, classes.size()) counts[classes[i]]++;
	u32 result = 0;
	for (u32 i=1; i<256; i++) result += counts[i] > 0 ? 1 : 0;
	return result;
}

bvec DatasetManager::GetFreeFlags()
{
	bvec res;
	FOR(i, flags.size()) res.push_back(flags[i] == _UNUSED);
	return res;
}
