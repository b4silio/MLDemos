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
	_TRAJ   = 0x1000,
	_OBST  = 0x10000,
	_TIME = 0x100000
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
	~RewardMap(){if(rewards) delete [] rewards;}
	RewardMap& operator= (const RewardMap& r);

	void SetReward(float *rewards, ivec size, fvec lowerBoundary, fvec higherBoundary);

	void Clear();

	void Zero();

	// return the value of the reward function at the coordinates provided
	float ValueAt(fvec sample);

	void SetValueAt(fvec sample, float value);

	void ShiftValueAt(fvec sample, float shift);

	void ShiftValueAt(fvec sample, float radius, float shift);

};

struct TimeSerie
{
	std::string name; // name of the current graph line
	std::vector<long int> timestamps; // time stamps for each frame
	std::vector<fvec> data; // each vector element is a frame
	TimeSerie(std::string name="", std::vector<long int> timestamps=std::vector<long int>(), std::vector<fvec> data=std::vector<fvec>()) : name(name), timestamps(timestamps), data(data){};
	bool operator==(const TimeSerie& t) const {
		if(name != t.name || timestamps.size() != t.timestamps.size() || data.size() != t.data.size()) return false;
		for(int i=0; i<timestamps.size(); i++) if(timestamps[i] != t.timestamps[i]) return false;
		for(int i=0; i<data.size(); i++) if(data[i] != t.data[i]) return false;
		return true;
	}
	TimeSerie& operator= (const TimeSerie& t)
	{
		if (this != &t) {
			name = t.name;
			timestamps = t.timestamps;
			data = t.data;
		}
		return *this;
	}
	fvec& operator[] (unsigned int i){return data[i];}
	fvec& operator() (unsigned int i){return data[i];}
	void clear(){data.clear();timestamps.clear();}
	size_t size(){return data.size();}
	std::vector<fvec>::iterator begin(){return data.begin();}
	std::vector<fvec>::iterator end(){return data.end();}

	TimeSerie& operator+=(const TimeSerie& t) {
		data.insert(data.end(), t.data.begin(), t.data.end());
		int count = timestamps.size();
		int lastTimestamp = timestamps.back();
		timestamps.insert(timestamps.end(), t.timestamps.begin(), t.timestamps.end());
		for(int i=count; i < timestamps.size(); i++) timestamps[i] += lastTimestamp;
		return *this;}
	TimeSerie operator+(const TimeSerie& t) const {TimeSerie a = *this; a+=t; return a;}
	TimeSerie& operator<< (const TimeSerie& t) {return *this += t;};

	TimeSerie& operator+=(const fvec& v) {data.push_back(v); timestamps.push_back(timestamps.back()+1); return *this;}
	TimeSerie operator+(const fvec& v) const {TimeSerie a = *this; a+=v; return a;}
	TimeSerie& operator<< (const fvec& v) {return *this += v;};
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

	std::vector<TimeSerie> series;

	RewardMap rewards;

	ivec labels;

	u32 *perm;

public:
    bool bProjected;

public:
	DatasetManager(int dimension = 2);
	~DatasetManager();

	void Randomize(int seed=-1);
	void Clear();
	double Compare(fvec sample);

	int GetSize(){return size;}
	int GetCount(){return samples.size();}
	int GetDimCount();
    std::pair<fvec, fvec> GetBounds();
	static u32 GetClassCount(ivec classes);

	// functions to manage samples
	void AddSample(fvec sample, int label = 0, dsmFlags flag = _UNUSED);
	void AddSamples(std::vector< fvec > samples, ivec newLabels=ivec(), std::vector<dsmFlags> newFlags=std::vector<dsmFlags>());
	void AddSamples(DatasetManager &newSamples);	
	void RemoveSample(unsigned int index);
    void RemoveSamples(ivec indices);

    fvec GetSample(int index=0){ return (index < samples.size()) ? samples[index] : fvec(); }
    fvec GetSampleDim(int index, ivec inputDims, int outputDim=-1);
    std::vector< fvec > GetSamples(){return samples;}
	std::vector< fvec > GetSamples(u32 count, dsmFlags flag=_UNUSED, dsmFlags replaceWith=_TRAIN);
    std::vector< fvec > GetSampleDims(ivec inputDims, int outputDim=-1);
	void SetSample(int index, fvec sample);
    void SetSamples(std::vector<fvec> samples){this->samples = samples;}

	int GetLabel(int index){return index < labels.size() ? labels[index] : 0;}
	ivec GetLabels(){return labels;}
	void SetLabel(int index, int label){if(index<labels.size())labels[index] = label;}
    void SetLabels(ivec labels){this->labels = labels;}

	// functions to manage sequences
	void AddSequence(int start, int stop);
	void AddSequence(ipair newSequence);
	void AddSequences(std::vector< ipair > newSequences);
	void RemoveSequence(unsigned int index);

	ipair GetSequence(unsigned int index){return index < sequences.size() ? sequences[index] : ipair(-1,-1);}
	std::vector< ipair > GetSequences(){return sequences;}
	std::vector< std::vector<fvec> > GetTrajectories(int resampleType, int resampleCount, int centerType, float dT, int zeroEnding);

	// functions to manage obstacles
	void AddObstacle(Obstacle o){obstacles.push_back(o);}
	void AddObstacle(fvec center, fvec axes, float angle, fvec power, fvec repulsion);
	void AddObstacles(std::vector<Obstacle> newObstacles);
	void RemoveObstacle(unsigned int index);
	std::vector< Obstacle > GetObstacles(){return obstacles;}
	Obstacle GetObstacle(unsigned int index){return index < obstacles.size() ? obstacles[index] : Obstacle();}

	// functions to manage rewards
	void AddReward(float *values, ivec size, fvec lowerBoundary, fvec higherBoundary);
	RewardMap *GetReward(){return &rewards;}

	// functions to manage time series
	void AddTimeSerie(std::string name, std::vector<fvec> data, std::vector<long int> timestamps=std::vector<long int>());
	void AddTimeSerie(TimeSerie serie);
	void AddTimeSeries(std::vector< TimeSerie > newTimeSeries);
	void RemoveTimeSerie(unsigned int index);
    std::vector<TimeSerie>& GetTimeSeries(){return series;}

	// functions to manage flags
	dsmFlags GetFlag(int index){return index < flags.size() ? flags[index] : _UNUSED;}
	void SetFlag(int index, dsmFlags flag){if(index < flags.size()) flags[index] = flag;}
	std::vector<dsmFlags> GetFlags(){return flags;}
	std::vector<bool> GetFreeFlags();
	void ResetFlags();

	void Save(const char *filename);
	bool Load(const char *filename);

};

#endif // _DATASET_MANAGER_H_
