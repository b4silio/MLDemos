#ifndef PCAPROJECTION_H
#define PCAPROJECTION_H

#include <public.h>
#include <mymaths.h>
#include <QLabel>
#include "basicOpenCV.h"

using namespace std;
using namespace cv;

class ProjectionPCA
{
    PCA pca;
public:
    std::vector<fvec> samples;
    ProjectionPCA();
    PCA compressPCA(const Mat& pcaset, int maxComponents, const Mat& testset, Mat& compressed);
    static IplImage *DrawEigenvals(Mat eigVal);
    void Train(std::vector<fvec> samples, int count=2);
    QLabel* EigenValues();
};

#endif // PCAPROJECTION_H
