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
#include <string.h>

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
    double *rewards;
	fvec lowerBoundary;
	fvec higherBoundary;
	RewardMap():rewards(0), dim(0), length(0){}
    ~RewardMap(){if(rewards) delete [] rewards; rewards=0;}
	RewardMap& operator= (const RewardMap& r);

    bool Empty() const {return length==0;}

    void SetReward(const double *rewards, const ivec size, const fvec lowerBoundary, const fvec higherBoundary);

    void SetReward(const float *rewards, const ivec size, const fvec lowerBoundary, const fvec higherBoundary);

	void Clear();

	void Zero();

	// return the value of the reward function at the coordinates provided
    float ValueAt(fvec sample) const ;

    float *GetRewardFloat() const ;

    void SetValueAt(const fvec sample, const double value);

    void ShiftValueAt(const fvec sample, const double shift);

    void ShiftValueAt(const fvec sample, const double radius, const double shift);
};

struct TimeSerie
{
	std::string name; // name of the current graph line
	std::vector<long int> timestamps; // time stamps for each frame
	std::vector<fvec> data; // each vector element is a frame
    TimeSerie(std::string name="", std::vector<long int> timestamps=std::vector<long int>(), std::vector<fvec> data=std::vector<fvec>()) : name(name), timestamps(timestamps), data(data){}
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
    fvec& operator[] (const unsigned int i) {return data[i];}
    fvec& operator() (const unsigned int i) {return data[i];}
    const fvec& at(const unsigned int i) const {return data.at(i);}
    void clear(){data.clear();timestamps.clear();}
    size_t size() const {return data.size();}
    std::vector<fvec>::iterator begin() {return data.begin();}
    std::vector<fvec>::iterator end() {return data.end();}

	TimeSerie& operator+=(const TimeSerie& t) {
		data.insert(data.end(), t.data.begin(), t.data.end());
		int count = timestamps.size();
		int lastTimestamp = timestamps.back();
		timestamps.insert(timestamps.end(), t.timestamps.begin(), t.timestamps.end());
		for(int i=count; i < timestamps.size(); i++) timestamps[i] += lastTimestamp;
		return *this;}
	TimeSerie operator+(const TimeSerie& t) const {TimeSerie a = *this; a+=t; return a;}
    TimeSerie& operator<< (const TimeSerie& t) {return *this += t;}

	TimeSerie& operator+=(const fvec& v) {data.push_back(v); timestamps.push_back(timestamps.back()+1); return *this;}
	TimeSerie operator+(const fvec& v) const {TimeSerie a = *this; a+=v; return a;}
    TimeSerie& operator<< (const fvec& v) {return *this += v;}
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
    std::map<int, std::vector<std::string> > categorical;

public:
    DatasetManager(const int dimension = 2);
	~DatasetManager();

    void Randomize(const int seed=-1);
	void Clear();
    double Compare(const fvec sample) const;

    int GetSize() const {return size;}
    int GetCount() const {return samples.size();}
    int GetDimCount() const;
    std::pair<fvec, fvec> GetBounds() const;
    static u32 GetClassCount(const ivec classes);

	// functions to manage samples
    void AddSample(const fvec sample, const int label = 0, const dsmFlags flag = _UNUSED);
    void AddSamples(const std::vector< fvec > samples, const ivec newLabels=ivec(), const std::vector<dsmFlags> newFlags=std::vector<dsmFlags>());
    void AddSamples(const DatasetManager &newSamples);
    void RemoveSample(const unsigned int index);
    void RemoveSamples(ivec indices);

    fvec GetSample(const int index=0) const { return (index < samples.size()) ? samples[index] : fvec(); }
    fvec GetSampleDim(const int index, const ivec inputDims, const int outputDim=-1) const;
    std::vector< fvec > GetSamples() const {return samples;}
    std::vector< fvec > GetSamples(const u32 count, const dsmFlags flag=_UNUSED, const dsmFlags replaceWith=_TRAIN);
    std::vector< fvec > GetSampleDims(const ivec inputDims, const int outputDim=-1) const ;
    std::vector< fvec > GetSampleDims(const std::vector<fvec> samples, const ivec inputDims, const int outputDim=-1) const ;
    void SetSample(const int index, const fvec sample);
    void SetSamples(const std::vector<fvec> samples){this->samples = samples;}

    int GetLabel(const int index) const {return index < labels.size() ? labels[index] : 0;}
    ivec GetLabels() const {return labels;}
	void SetLabel(int index, int label){if(index<labels.size())labels[index] = label;}
    void SetLabels(ivec labels){this->labels = labels;}

    std::string GetCategorical(const int dimension,const  int value) const ;
    bool IsCategorical(const int dimension) const ;

	// functions to manage sequences
    void AddSequence(const int start, const int stop);
    void AddSequence(const ipair newSequence);
    void AddSequences(const std::vector< ipair > newSequences);
    void RemoveSequence(const unsigned int index);

    ipair const GetSequence(const unsigned int index) const {return index < sequences.size() ? sequences[index] : ipair(-1,-1);}
    std::vector< ipair > GetSequences() const {return sequences;}
    std::vector< std::vector<fvec> > GetTrajectories(const int resampleType, const int resampleCount, const int centerType, const float dT, const int zeroEnding) const ;

	// functions to manage obstacles
    void AddObstacle(const Obstacle o){obstacles.push_back(o);}
    void AddObstacle(const fvec center, const fvec axes, const float angle, const fvec power, const fvec repulsion);
    void AddObstacles(const std::vector<Obstacle> newObstacles);
    void RemoveObstacle(const unsigned int index);
    std::vector< Obstacle > GetObstacles() const {return obstacles;}
    Obstacle GetObstacle(const unsigned int index) const {return index < obstacles.size() ? obstacles[index] : Obstacle();}

	// functions to manage rewards
    void AddReward(const float *values, const ivec size, const fvec lowerBoundary, const fvec higherBoundary);
    RewardMap *GetReward() {return &rewards;}

	// functions to manage time series
    void AddTimeSerie(const std::string name, const std::vector<fvec> data, const std::vector<long int> timestamps=std::vector<long int>());
    void AddTimeSerie(const TimeSerie serie);
    void AddTimeSeries(const std::vector< TimeSerie > newTimeSeries);
    void RemoveTimeSerie(const unsigned int index);
    std::vector<TimeSerie>& GetTimeSeries() {return series;}

	// functions to manage flags
    dsmFlags GetFlag(const int index) const {return index < flags.size() ? flags[index] : _UNUSED;}
    void SetFlag(const int index, const dsmFlags flag){if(index < flags.size()) flags[index] = flag;}
    std::vector<dsmFlags> GetFlags() const {return flags;}
    std::vector<bool> GetFreeFlags() const ;
	void ResetFlags();

    void Save(const char *filename);
	bool Load(const char *filename);
};

#endif // _DATASET_MANAGER_H_
