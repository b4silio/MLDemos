#ifndef _EIGEN_FACES_H_
#define _EIGEN_FACES_H_

#include <vector>

class EigenFaces
{
private:
	int dim;
	int trainCnt;
	IplImage **eigenVectors;
	IplImage *avgImage;
	CvMat *eigenValues;
	std::vector<float *> projections;
	std::vector<int> classes;
	std::vector<bool> isTraining;
	bool bUseColor;

	int FindNearestNeighbor(float *candidate);

public:
	EigenFaces();
	~EigenFaces();
	void Learn(std::vector<IplImage *> faces, std::vector<int> classes, std::vector<bool> isTrainingData=std::vector<bool>(), bool bColor = true);
	void Recognize(IplImage *face);
	void Draw(bool bMonochrome=false, int e1 = 0, int e2 = 1);
    IplImage *DrawEigenVals();
	std::vector<IplImage *> GetEigenVectorsImages();

	std::vector<float *> GetProjections(int dim = 0, bool bNormalized=false);
	std::vector<int> GetClasses(){return classes;};
	IplImage **GetVectors(){return eigenVectors;};
	int GetVectorCount(){return dim;};

	IplImage *mapImage;

};
#endif // _EIGEN_FACES_H_
