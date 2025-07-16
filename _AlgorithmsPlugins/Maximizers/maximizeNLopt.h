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
    std::vector<GLObject> DrawGL();
    void Train(float *dataMap, fVec size, fvec startingPoint=fvec());
    fvec Test( const fvec &sample);
    fvec Test(const fVec &sample);
    const char *GetInfoString();
    void SetParams(Maximizer *maximizer, fvec parameters);
    fvec GetParams();
    void GetParameterList(std::vector<QString> &parameterNames,
                                 std::vector<QString> &parameterTypes,
                                 std::vector< std::vector<QString> > &parameterValues);
};

#endif // MAXIMIZENLOPT_H
