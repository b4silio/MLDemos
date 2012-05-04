#ifndef _GA_PEON_H_
#define _GA_PEON_H_

#include <public.h>
#include <vector>

enum FitnessType
{
	FITNESS_BIC,
	FITNESS_GMR
};

class GAPeon
{
private:
	u32 dim;
	float *dna;
    int type; // 0: continuous, 1: discrete 8-way, 2: discrete 4-way
public:
    GAPeon(u32 dim=2, int type=0);
	GAPeon( const GAPeon &peon );
	~GAPeon();

	inline GAPeon & operator = ( const GAPeon &peon )
	{
		if(this == &peon) return *this;
		dim = peon.dim;
        type = peon.type;
        KILL(dna);
		dna = new float[dim];
		std::copy(peon.dna, peon.dna+dim, dna);
		return *this;
	}

	inline bool operator == ( const GAPeon &peon ) const
	{
		if(peon.dim != dim) return false;
		FOR(i, dim) if(dna[i] != peon.dna[i]) return false;
		return true;
	}

    static GAPeon Random(u32 dim, int type=0);
    void Randomize();
	void Mutate(f32 alpha = 0.01f);
    std::pair<GAPeon,GAPeon> Cross(GAPeon peon, float alpha=0.1f);

    float *Dna(){return dna;}
    u32 Count(){return dim;}

	double Fitness(float *data, int w, int h);
	fvec ToSample();
};

#endif // _GA_PEON_H_
