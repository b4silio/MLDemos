/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#ifndef _DATASET_MANAGER_H_
#define _DATASET_MANAGER_H_

#include <vector>
#include "public.h"

enum DatasetManagerFlags
{
	_UNUSED = 0x0000,
	_TRAIN  = 0x0001,
	_VALID  = 0x0010,
	_TEST   = 0x0100,
	_TRAJ = 0x1000,
	_OBST= 0x10000
};
typedef DatasetManagerFlags dsmFlags;

struct Obstacle
{
	fvec axes;			//the obstacle major axes
	fvec center;		//the center of the obstacle
	float angle;		//the orientation matrix
	fvec power;			//Gamma is \sum( (x/a)^m )
	fvec repulsion;	//safety factor
	Obstacle() :angle(0) {
		axes.resize(2,1.f);
		center.resize(2,0.f);
		power.resize(2,1.f);
		repulsion.resize(2,1.f);
	};
	bool operator==(const Obstacle& o) const {
		return center == o.center && axes == o.axes && angle == o.angle && power == o.power && repulsion == o.repulsion;
	}
	bool operator!=(const Obstacle& o) const {
		return center != o.center || axes != o.axes || angle != o.angle || power != o.power || repulsion != o.repulsion;
	}
};

struct RewardMap
{
	int dim;
	ivec size; // size of reward array in each dimension
	int length; // size[0]*size[1]*...*size[dim]
	float *rewards;
	fvec lowerBoundary;
	fvec higherBoundary;
	RewardMap():rewards(0), dim(0), length(0){}
	~RewardMap(){if(rewards) delete [] rewards;};
	RewardMap& operator= (const RewardMap &r) {
	  if (this != &r) {
		  dim = r.dim;
		  size = r.size;
		  length = r.length;
		  lowerBoundary = r.lowerBoundary;
		  higherBoundary = r.higherBoundary;
		  if(rewards) delete [] rewards;
		  rewards = new float[length];
		  memcpy(rewards, r.rewards, length*sizeof(float));
	  }
	  return *this;
	}

	void SetReward(float *rewards, ivec size, fvec lowerBoundary, fvec higherBoundary)
	{
		this->lowerBoundary = lowerBoundary;
		this->higherBoundary = higherBoundary;
		this->size = size;
		dim = size.size();
		length = 1;
		FOR(i, size.size()) length *= size[i];
		if(this->rewards) delete [] this->rewards;
		this->rewards = new float[length];
		memcpy(this->rewards, rewards, length*sizeof(float));
	}

	void Clear()
	{
		dim = 0;
		size.clear();
		length = 0;
		lowerBoundary.clear();
		higherBoundary.clear();
		if(rewards) delete [] rewards;
	}

	void Zero()
	{
		FOR(i, length) rewards[i] = 0;
	}

	// return the value of the reward function at the coordinates provided
	float ValueAt(fvec sample)
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

	void SetValueAt(fvec sample, float value)
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

	void ShiftValueAt(fvec sample, float shift)
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

	void ShiftValueAt(fvec sample, float radius, float shift)
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
};

class DatasetManager
{
protected:
	static u32 IDCount;

	u32 ID;

	int size; // the samples size (dimension)

	std::vector< fvec > samples;

	std::vector< ipair > sequences;

	std::vector<dsmFlags> flags;

	std::vector<Obstacle> obstacles;

	RewardMap rewards;

	ivec labels;

	u32 *perm;

public:
	DatasetManager(int dimension = 2);

	~DatasetManager();

	void Clear();

	void Randomize(int seed=-1);

	void AddSample(fvec sample, int label = 0, dsmFlags flag = _UNUSED);

	void AddSamples(std::vector< fvec > samples, ivec newLabels=ivec(), std::vector<dsmFlags> newFlags=std::vector<dsmFlags>());

	void AddSamples(DatasetManager &newSamples);

	void AddSequence(int start, int stop);

	void AddSequence(ipair newSequence);

	void AddSequences(std::vector< ipair > newSequences);

	void RemoveSequence(unsigned int index);

	void RemoveSample(unsigned int index);

	double Compare(fvec sample);

	int GetSize(){return size;};

	int GetCount(){return samples.size();};

	fvec GetSample(int index=0){return index<samples.size() ? samples[index] : fvec();};

	std::vector< fvec > GetSamples(){return samples;};

	std::vector< fvec > GetSamples(u32 count, dsmFlags flag=_UNUSED, dsmFlags replaceWith=_TRAIN);

	ipair GetSequence(unsigned int index){return index < sequences.size() ? sequences[index] : ipair(-1,-1);};

	std::vector< ipair > GetSequences(){return sequences;};

	int GetLabel(int index){return index < labels.size() ? labels[index] : 0;};

	ivec GetLabels(){return labels;};

	void SetLabel(int index, int label){if(index<labels.size())labels[index] = label;};

	void AddObstacle(Obstacle o){obstacles.push_back(o);};

	void AddObstacle(fvec center, fvec axes, float angle, fvec power, fvec repulsion);

	void AddObstacles(std::vector<Obstacle> newObstacles);

	void RemoveObstacle(unsigned int index);

	std::vector< Obstacle > GetObstacles(){return obstacles;};

	Obstacle GetObstacle(unsigned int index){return index < obstacles.size() ? obstacles[index] : Obstacle();};

	void AddReward(float *values, ivec size, fvec lowerBoundary, fvec higherBoundary);

	RewardMap *GetReward(){return &rewards;};

	void ResetFlags();

	dsmFlags GetFlag(int index){return index < flags.size() ? flags[index] : _UNUSED;};

	void SetFlag(int index, dsmFlags flag){if(index < flags.size()) flags[index] = flag;};

	std::vector<dsmFlags> GetFlags(){return flags;};

	std::vector<bool> GetFreeFlags();

	void Save(const char *filename);

	bool Load(const char *filename);

	static u32 GetClassCount(ivec classes);

};

#endif // _DATASET_MANAGER_H_
