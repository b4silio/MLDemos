#ifndef PCAPROJECTION_H
#define PCAPROJECTION_H

#include <public.h>
#include <mymaths.h>
#include "basicOpenCV.h"

using namespace std;
using namespace cv;

class PCAProjection
{
public:
    std::vector<fvec> samples;
    PCAProjection();
    PCA compressPCA(const Mat& pcaset, int maxComponents, const Mat& testset, Mat& compressed);
    void Train(std::vector<fvec> samples, int count=2);
};

#endif // PCAPROJECTION_H
