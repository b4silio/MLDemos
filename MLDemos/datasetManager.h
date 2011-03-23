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
