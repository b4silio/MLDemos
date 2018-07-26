#ifndef _EIGEN_FACES_H_
#define _EIGEN_FACES_H_

#include <public.h>
#include <vector>

class EigenFaces
{
private:
    int                     dim;
    int                     trainCnt;
    std::vector<cv::Mat>    eigenVectors;
    cv::Mat                 avgImage;
    cv::Mat                 eigenValues;
    std::vector<float *>    projections;
    std::vector<int>        classes;
    std::vector<bool>       isTraining;
    bool                    bUseColor;
    cv::PCA                 pca;

    int                     FindNearestNeighbor(float *candidate);

public:
	EigenFaces();
	~EigenFaces();
    void                    Learn(std::vector<cv::Mat> faces, std::vector<int> classes, std::vector<bool> isTrainingData=std::vector<bool>(), bool bColor = true);
    IplImage *              DrawEigenVals();
    std::vector<cv::Mat>    GetEigenVectorsImages(int heatmapMode=0);
    fvec                    GetEigenValues();
    cv::Vec3b               ValueToColor(float value, bool bSimple=false);

    std::vector<float *>    GetProjections(int dim = 0, bool bNormalized=false);
    std::vector<int>        GetClasses(){return classes;}
    std::vector<cv::Mat>    GetVectors(){return eigenVectors;}
    int                     GetVectorCount(){return dim;}
};
#endif // _EIGEN_FACES_H_
