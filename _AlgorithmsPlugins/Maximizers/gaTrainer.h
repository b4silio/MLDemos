#ifndef _GA_TRAINER_H_
#define _GA_TRAINER_H_

#include <vector>
#include "gaPeon.h"

class GATrain
{
private:
	std::vector<GAPeon> population;
	std::vector<double> fitness;
	std::vector<double> oldFitness;
	GAPeon best;
	u32 dim;
	f32 alphaMute;
	f32 alphaCross;
	f32 alphaSurvivors;
	double bestFitness;
	double meanFitness;
	u32 popSize;
	float *data;
	int w, h;
public:
	GATrain(float *data, int w, int h, int populationSize=50, int dim=2);
	void Generate(u32 count);
	void Kill(u32 index);
	void NextGen();

	f32 AlphaMute(){return alphaMute;};
	f32 AlphaSurvivors(){return alphaSurvivors;};
	f32 AlphaCross(){return alphaCross;};
	void AlphaMute(f32 alpha){alphaMute = alpha;};
	void AlphaSurvivors(f32 alpha){alphaSurvivors = alpha;};
	void AlphaCross(f32 alpha){alphaCross = alpha;};

	double BestFitness(){return bestFitness;};
	double MeanFitness(){return meanFitness;};
	int GetBest();
	GAPeon &Best(){return best;};

	std::vector<double> &Fitness(){return oldFitness;};
	std::vector<GAPeon> &Population(){return population;};
};

#endif // _GA_TRAINER_H_
