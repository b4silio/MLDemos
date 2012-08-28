#ifndef REINFORCEMENTPROBLEM_H
#define REINFORCEMENTPROBLEM_H

#include <vector>
#include "public.h"
#include "mymaths.h"
#include <QPainter>

class ReinforcementProblem
{
    u32 dim;
    int w, h;
    float *data;

public:
    int policyType;
    int gridSize;
    int quantizeType;
    fvec directions, tempDirections;
    fvec stateValues;
    int simulationSteps, problemType, displayIterationsCount, rewardType;

    ReinforcementProblem();
    ~ReinforcementProblem();
    static inline float GetValue(fvec sample, float *data, int w, int h)
    {
        int xIndex = max(0, min(w-1, (int)(sample[0]*w)));
        int yIndex = max(0, min(h-1, (int)(sample[1]*h)));
        int index = yIndex*w + xIndex;
        return data[index];
    }
    float GetValue(fvec sample);
    void SetValue(fvec sample, float value);
    float GetSimulationValue(fvec sample);

    // use the policy to decide which action to take, and perform the action
    inline fvec GetDeltaAt(int x, int y, fvec &directions);
    fvec NextStep(fvec sample, fvec directions);
    fvec PerformAction(fvec sample);
    float GetReward();
    float GetReward(fvec directions);
    void Draw(QPainter &painter);
    void Initialize(float *dataMap, fVec size, fvec startingPoint=fvec());
};

#endif // REINFORCEMENTPROBLEM_H
