#include <public.h>
#include <basicMath.h>
#include <mymaths.h>
#include <algorithm>
#include "gaTrainer.h"

using namespace std;
/************************************************************************/
/*                     Genetic Algorithm Individual                     */
/************************************************************************/

GAPeon::GAPeon(u32 dim)
: dim(dim), dna(0)
{
	dna = new float[dim];
	FOR(d, dim) dna[d] = 0.;
}

GAPeon::GAPeon( const GAPeon &peon )
{
	dim = peon.dim;
	dna = new float[dim];
	memcpy(dna, peon.dna, dim*sizeof(float));
}

GAPeon::~GAPeon()
{
	KILL(dna);
}

GAPeon GAPeon::Random(u32 dim)
{
	GAPeon peon(dim);
	FOR(d, dim) peon.Dna()[d] = drand48();
	return peon;
}

void GAPeon::Randomize()
{
	FOR(d, dim) dna[d] = drand48();
}

void GAPeon::Mutate(f32 alpha)
{
	bool bBitWise = false;

	if(bBitWise)
	{
		u32 *_dna = reinterpret_cast<u32*>(dna);
		if(!_dna) return;
		FOR(i, dim)
		{
			FOR(j,sizeof(u32))
			{
				if(drand48() < alpha)
				{
					_dna[j] = (_dna[j] ^ ~(0x1<<j)) | (~_dna[j] ^ (0x1<<j));
				}
			}
		}
	}
	else
	{
		FOR(d, dim)
		{
			dna[d] += (drand48()*2.f-1.f)*alpha;
			dna[d] = max(0.f, min(1.f, dna[d]));
		}
	}
}

pair<GAPeon,GAPeon> GAPeon::Cross(GAPeon peon)
{
	u32 crossPoint = rand()%(dim*sizeof(u32)-2)+1;
	u32 crossIndex = crossPoint % sizeof(u32);
	u32 crossDim = crossPoint / sizeof(u32);
	GAPeon baby1(*this);
	GAPeon baby2(peon);

	u32 *dna1 = reinterpret_cast<u32*>(dna);
	u32 *dna2 = reinterpret_cast<u32*>(peon.Dna());
	u32 *newDna1 = reinterpret_cast<u32*>(baby1.Dna());
	u32 *newDna2 = reinterpret_cast<u32*>(baby2.Dna());

	u32 mask = (0x1<<crossIndex+1)-1;
	newDna1[crossDim] = dna1[crossDim] & ~mask | dna2[crossDim] & mask;
	newDna2[crossDim] = dna2[crossDim] & ~mask | dna1[crossDim] & mask;
	for(int i=crossDim+1; i<dim; i++)
	{
		newDna1[i] = dna2[i];
		newDna2[i] = dna1[i];
	}
	return pair<GAPeon,GAPeon>(baby1,baby2);
}

fvec GAPeon::ToSample()
{
	fvec sample;
	sample.resize(dim,0);
	FOR(d, dim) sample[d] = dna[d];
	return sample;
}

#include <QDebug>

double GAPeon::Fitness( float *data, int w, int h)
{
	if(!data) return 0;
	int x = (int)(dna[0] * w);
	int y = (int)(dna[1] * h);
	x = max(0,min(w-1,x));
	y = max(0,min(h-1,y));
//	qDebug() << "sample: " << x << ":" << y << " Fitness: " << data[y*w + x];
	return data[y*w + x];
}
