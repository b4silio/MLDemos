#include <public.h>
#include "basicMath.h"
#include "basicOpenCV.h"
#include "eigenFaces.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/ml/ml.hpp>
#include <cmath>


EigenFaces::EigenFaces()
{
	bUseColor = false;
}

EigenFaces::~EigenFaces()
{
	FOR(i, projections.size()) delete [] projections[i];
	projections.clear();
}

std::vector<float *> EigenFaces::GetProjections(int dim, bool bNormalized)
{
	if(!dim || dim >= this->dim) return projections;
	std::vector <float *> result;
	FOR(i, projections.size())
	{
		float *r = new float[dim];
		FOR(j, dim) r[j] = projections[i][j];
		result.push_back(r);
	}

	// we return all values in a 0-1 range
	if (bNormalized)
	{
		// we find min and max values for each dimension
		FOR(i, dim)
		{

			float minX, maxX;
			minX = FLT_MAX;
			maxX = FLT_MIN;
			float dX;

			FOR(j, result.size())
			{
				if(minX > result[j][i]) minX = result[j][i];
				if(maxX < result[j][i]) maxX = result[j][i];
			}
			dX = maxX - minX;

			FOR(j, result.size())
			{
				result[j][i] = (result[j][i] - minX) / dX;
			}
		}
	}

	return result;
}

// Create data matrix from a vector of images
static  cv::Mat createDataMatrix(const std::vector<cv::Mat> &images, bool bColor)
{
  // Allocate space for all images in one data matrix.
  // The size of the data matrix is
  //
  // ( w  * h * 1|3, numImages )
  //
  // where,
  //
  // w = width of an image in the dataset.
  // h = height of an image in the dataset.
  // 1|3 = number of color channels.
  // numImages = number of images in the dataset.

  cv::Mat data(static_cast<int>(images.size()), images[0].rows * images[0].cols * (bColor?3:1), CV_32F);

  // Turn an image into one row vector in the data matrix
  for(unsigned int i = 0; i < images.size(); i++)
  {
    // Extract image as one long vector of size w x h (x 3 if color)
    cv::Mat image = images[i].reshape(1,1);

    // Copy the long vector into one row of the destm
    image.copyTo(data.row(i));

  }
  return data;
}

void EigenFaces::Learn(std::vector<cv::Mat> faces, std::vector<int> classes, std::vector<bool> isTrainingData, bool bColor)
{
    if(faces.empty() || !faces[0].cols) return;
	bUseColor = bColor;

	//this->classes = classes;
	isTraining.clear();
	this->classes.clear();
	FOR(i, classes.size())
	{
		//if(classes[i] == 0) continue;
		this->classes.push_back(classes[i]);
		if(i<isTrainingData.size())
			isTraining.push_back(isTrainingData[i]);
		else isTraining.push_back(true);
	}
	if(this->classes.size() == 0) return; // no labeled data!

    cv::Size res = faces[0].size();
	trainCnt = this->classes.size();
	    
	FOR(i, faces.size())
	{
        cv::Mat& face = faces.at(i);
		//if(classes[i] == 0) continue;
		if(!bUseColor)
		{
            if(face.channels() == 3) {
                cv::Mat grayFace;
                cv::cvtColor(face, grayFace, cv::COLOR_BGR2GRAY);
                cv::equalizeHist(grayFace, grayFace);
                faces[i] = grayFace;
			}
        } else {
            if(face.channels() == 1) {
                cv::Mat colorFace;
                cv::cvtColor(face, colorFace, cv::COLOR_GRAY2BGR);
                faces[i] = colorFace;
            }
        }
	}
    cv::Mat trainingData = createDataMatrix(faces, bColor);

	FOR(i, isTraining.size()) if(!isTraining[i]) trainCnt--;
	dim = trainCnt - 1;

    pca = cv::PCA(trainingData, cv::Mat(), cv::PCA::DATA_AS_ROW, dim);

    cv::Mat eigenVects = pca.eigenvectors;
    eigenValues = pca.eigenvalues;

    eigenVectors.clear();
    FOR(i, eigenVects.rows) {
        eigenVectors.push_back(eigenVects.row(i).reshape(bColor ? 3 : 1, res.height));
    }

    avgImage = pca.mean.reshape(bColor ? 3 : 1, res.height);

	// we compute the projections for recognition purposes
	FOR(i, projections.size()) delete [] projections[i];
	projections.clear();
	FOR(i, this->classes.size())
	{
        float *proj = new float[dim];
        cv::Mat faceAsVector = faces[i].reshape(1,1);
        cv::Mat projection = pca.project(faceAsVector);
        FOR(j, dim) proj[j] = projection.at<float>(j);
		projections.push_back(proj);
	}
}

int EigenFaces::FindNearestNeighbor(float *candidate)
{
	double minDist = DBL_MAX;
    int nearest = 0;
	bool bMahalanobis = true;

	FOR(train, trainCnt)
	{
		double dist = 0;
		FOR(i, dim)
		{
			float d = candidate[i] - projections[train][i];
			dist += d*d;
            if(bMahalanobis) dist /= eigenValues.at<float>(i);
		}

		if(dist < minDist)
		{
			minDist = dist;
			nearest = train;
		}
	}

	return nearest;
}

int eigparams[7];

cv::Vec3b EigenFaces::ValueToColor(float value, bool bSimple)
{
    if(bSimple) {
        value = (value-0.5f)*5.f*255.f;
        if(value > 0) return cv::Vec3b(0, value/2, value);
        else return cv::Vec3b(value/2,value,0);
    } else {
        value = (value-0.5)*4 + 0.5;
        float a = (1-value)/0.2;
        int X = int(a);
        int Y = int(255*(a-X));
        int r=0,g=0,b=0;
        switch(X)
        {
            case 0: r=255;g=Y;b=0;break;
            case 1: r=255-Y;g=255;b=0;break;
            case 2: r=0;g=255;b=Y;break;
            case 3: r=0;g=255-Y;b=255;break;
            case 4: r=Y;g=0;b=255;break;
            case 5: r=255;g=0;b=255;break;
        }
        return cv::Vec3b(b,g,r);
    }
}

std::vector<cv::Mat> EigenFaces::GetEigenVectorsImages(int heatmapMode)
{
    std::vector<cv::Mat> result;
    bool bColor = eigenVectors.size() && eigenVectors[0].channels() > 1;
    cv::Mat accumulator = cv::Mat(eigenVectors[0].size(), bColor ? CV_32FC3 : CV_32F);

    FOR(i, dim-2) // dim -2 because for some reasons the last two eigenvalues are often fubared
	{
        accumulator = cv::Mat::zeros(eigenVectors[0].size(), bColor ? CV_32FC3 : CV_32F);
        cv::addWeighted(eigenVectors[i],1,accumulator,1,0,accumulator);
        cv::normalize(accumulator, accumulator, 0, 255, cv::NORM_MINMAX, CV_8UC1);
        cv::Mat newEigenVec = cv::Mat(eigenVectors[0].size(), bColor ? CV_8UC3 : CV_8UC1);
        cv::convertScaleAbs(accumulator, newEigenVec);

        if(heatmapMode > 0) {
            cv::Mat heatmap = cv::Mat(newEigenVec.size(), CV_8UC3);
            FOR(j, heatmap.cols*heatmap.rows) {
                float value;
                if(!bColor) value = newEigenVec.at<uchar>(j) / 255.f;
                else {
                    cv::Vec3b c = newEigenVec.at<cv::Vec3b>(j);
                    value = ((c[0] + c[1] + c[2]) / 3.f) / 255.f;
                }
                heatmap.at<cv::Vec3b>(j) = ValueToColor(value, heatmapMode == 1);
            }
            newEigenVec = heatmap;
        }

        cv::Mat larger;
        cv::resize(newEigenVec,larger, cv::Size(128,128),0,0,cv::INTER_CUBIC);
        result.push_back(larger);
	}
	return result;
}

fvec EigenFaces::GetEigenValues()
{
    fvec eigens(eigenValues.rows);
    for(int i=0; i<eigenValues.rows; i++) {
        eigens[i] = eigenValues.at<float>(i);
    }
    return eigens;
}
