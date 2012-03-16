#include <public.h>
#include "gaTrainer.h"
#include <basicMath.h>
#include <mymaths.h>
#include <algorithm>

using namespace std;
/************************************************************************/
/*                 Genetic Algorithm Training Procedure                 */
/************************************************************************/

GATrain::GATrain(float *data, int w, int h, int populationSize, int dim)
:	dim(dim), popSize(populationSize), data(data), w(w), h(h),
	alphaMute(0.01f), alphaCross(0.5f), alphaSurvivors(0.2f),
	bestFitness(0), meanFitness(0), best(GAPeon(dim))
{
}

void GATrain::Generate( u32 count )
{
	popSize = count;
	population.clear();
	fitness.clear();
	oldFitness.clear();
	bestFitness = meanFitness = 0;
	FOR(i, count)
	{
		GAPeon peon = GAPeon::Random(dim);
		population.push_back(peon);
		fitness.push_back(0);
	}
	best = population[GetBest()];
}

int GATrain::GetBest()
{
	double fit = fitness[0];
	int best = 0;
	FOR(i, fitness.size())
	{
		if(fit < fitness[i])
		{
			best = i;
			fit = fitness[i];
		}
	}
	return best;
}

void GATrain::NextGen()
{
	// we compute the fitness for each peon
	FOR(i, population.size())
	{
		fitness[i] = population[i].Fitness(data, w, h);
	}

	std::vector< std::pair<double, u32> > fits;
	FOR(i, fitness.size()) fits.push_back(std::pair<double, u32>(fitness[i], i));
	std::sort(fits.begin(), fits.end(), std::greater< std::pair<double,u32> >());
	std::vector<GAPeon> newPop;
	std::vector<double> newFits;
	FOR(i, max(1,(int)(fits.size()*alphaSurvivors)))
	{
		newPop.push_back(population[fits[i].second]);
		newFits.push_back(fitness[fits[i].second]);
	}
	for(int i=newPop.size(); i<popSize; i++)
	{
		newPop.push_back(GAPeon::Random(dim));
		newFits.push_back(0);
	}
	population = newPop;
	fitness = newFits;

	double maxFitness = fitness[0];

	meanFitness = 0;
	u32 cnt = 0;
	FOR(i, fitness.size())
	{
		if(fitness[i] < -1e8) continue;
		meanFitness += fitness[i];
		//fitness[i] += drand48() * 0.01f; // we add some noise to ensure that we pick 0-valued elements
		cnt++;
	}
	meanFitness /= (double)cnt;
	best = population[0];
	bestFitness = fitness[0];
	oldFitness = fitness;


	// we normalize probabilities by fitness value
	double fitnessSum = 0;
	FOR(i, fitness.size()) fitnessSum += fitness[i]*6.f + 4.f;
	vector<double> probs;
	double fitnessCounter = 0;
	FOR(i, fitness.size())
	{
		fitnessCounter += (fitness[i]*6.f + 4.f)/fitnessSum;
		probs.push_back(fitnessCounter);
	}

	// we generate the new population
	newPop.clear();
	newFits.clear();
	FOR (i, max(1.f,alphaSurvivors*popSize))
	{

		/*
		// copy the elite from the old population
		if(i < (u32)(alphaSurvivors*popSize)/2)
		{
			newPop.push_back(population[i]);
			newFits.push_back(fitness[i]);
			continue;
		}
		*/

		// we select two parents depending on their probability
		u32 mom, dad, j;
		double r = drand48();
		for (j=0; j<probs.size() && r < probs[j]; j++);
		mom = j<probs.size() ? j : 0;
		r = drand48();
		for (u32 j=0; j<probs.size() && r < probs[j]; j++);
		dad = j<probs.size() ? j : probs.size()-1;
		//mom = rand() % population.size();
		//dad = rand() % population.size();
		if(mom == dad) dad = mom < probs.size()-1 ? mom+1 : mom>0 ? mom-1 : 0;

		// let's make some babies...
		pair<GAPeon, GAPeon> babies;
		if(drand48() < alphaCross)
		{
			babies = population[mom].Cross(population[dad]);
		}
		else // it's them clones!
		{
			babies.first = population[mom];
			babies.second = population[dad];
		}

		// and we add these guys to the new population
		newPop.push_back(babies.first);
		newPop.push_back(babies.second);
		newFits.push_back(-1e10);
		newFits.push_back(-1e10);
		i++;
	}
	FOR(i, newPop.size()) newPop[i].Mutate(alphaMute);

	population = newPop;
	fitness = newFits;

	// we look for duplicates
	u32 maxClones = 2;
	u32 deaders = 0;
	FOR(i, population.size()-1)
	{
		u32 clones = 0;
		for(u32 j=i+1; j<population.size(); j++)
		{
			if(population[i] == population[j])
			{
				clones++;
				if(clones > maxClones)
				{
					Kill(j);
					j--;
					deaders++;
				}
			}
		}
	}
	//printf("clones killed: %d\n", deaders);

	// we replace the pruned clones with random offspring
	FOR(i, deaders)
	{
		newPop.push_back(GAPeon::Random(dim));
		newFits.push_back(-1e10);
	}
}

void GATrain::Kill( u32 index )
{
	if(index >= fitness.size()) return;
	for (u32 i=index; i<fitness.size()-1; i++)
	{
		population[i] = population[i+1];
		fitness[i] = fitness[i+1];
	}
	population.pop_back();
	fitness.pop_back();
}
