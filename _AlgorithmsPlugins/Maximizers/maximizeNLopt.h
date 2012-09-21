#ifndef MAXIMIZENLOPT_H
#define MAXIMIZENLOPT_H

#include <maximize.h>
#include <nlopt/nlopt.hpp>

class MaximizeNlopt : public Maximizer
{
    int type;
    float step;
public:
    static std::vector<fvec> evaluationList;
    static int evaluationFrame;

    MaximizeNlopt();
    ~MaximizeNlopt();

    void SetParams(int type, float step);

    void Draw(QPainter &painter);
    void Train(float *dataMap, fVec size, fvec startingPoint=fvec());
    fvec Test( const fvec &sample);
    fvec Test(const fVec &sample);
    const char *GetInfoString();
};

#endif // MAXIMIZENLOPT_H
