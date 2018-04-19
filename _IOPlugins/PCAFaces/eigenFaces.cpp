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

    eigenValues = cv::Mat(1,dim,CV_32FC1);
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
	int train = 0, nearest = 0;
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

std::vector<cv::Mat> EigenFaces::GetEigenVectorsImages()
{
    std::vector<cv::Mat> result;
    bool bColor = eigenVectors.size() && eigenVectors[0].channels() > 1;
    cv::Mat accumulator = cv::Mat(eigenVectors[0].size(), bColor ? CV_32FC3 : CV_32F);
    //IplImage *acc = cvCreateImage(cvSize(eigenVectors[0].cols,eigenVectors[0].rows), IPL_DEPTH_32F, 1);
	FOR(i, dim-2) // dim -2 because for some reasons the last two eigenvalues are often fubared
	{
        accumulator = cv::Mat::zeros(eigenVectors[0].size(), bColor ? CV_32FC3 : CV_32F);
        cv::addWeighted(eigenVectors[i],1,accumulator,1,0,accumulator);
        cv::normalize(accumulator, accumulator, 0, 255, cv::NORM_MINMAX, CV_8UC1);
        cv::Mat newEigenVec = cv::Mat(eigenVectors[0].size(), bColor ? CV_8UC3 : CV_8UC1);
        cv::convertScaleAbs(accumulator, newEigenVec);

        cv::Mat larger;
        cv::resize(newEigenVec,larger, cv::Size(128,128),0,0,cv::INTER_CUBIC);
        result.push_back(larger);
	}
	return result;
}

IplImage *EigenFaces::DrawEigenVals()
{
    IplImage *eigImage = cvCreateImage(cvSize(440,440),8,3);
	cvSet(eigImage, CV_RGB(255,255,255));

	float maxEigVal = 0;
    FOR(i, dim-2) if(!std::isnan(eigenValues.at<float>(i))) maxEigVal += eigenValues.at<float>(i);
	float accumulator = 0;
    maxEigVal = max(1.f,maxEigVal);

	cvDrawLine(eigImage, cvPoint(0, eigImage->height-1), cvPoint(eigImage->width, eigImage->height-1), CV_RGB(180,180,180));
	cvDrawLine(eigImage, cvPoint(0,0), cvPoint(0, eigImage->height), CV_RGB(180,180,180));
	CvPoint point = cvPoint(0,0);
    FOR(i, dim-2) { // dim -2 because for some reasons the last two eigenvalues are often fubared
        float eigVal = eigenValues.at<float>(i);
        if(!std::isnan(eigVal)) {
            CvPoint point2 = cvPoint(i * eigImage->width / dim, eigImage->height - (int)(eigVal / maxEigVal * eigImage->height));
            //cvDrawCircle(eigImage, point, 1, CV_RGB(0,0,0), -1, CV_AA);
            cvDrawLine(eigImage, point, point2, CV_RGB(0,0,0));
            accumulator += eigVal / maxEigVal;
            point = point2;
        }
    }
	cvDrawLine(eigImage, point, cvPoint(eigImage->width, eigImage->height), CV_RGB(0,0,0));

    IplImage *display = cvCreateImage(cvSize(eigImage->width+40,eigImage->height+40),8,3);
	cvSet(display, CV_RGB(255,255,255));
	ROI(display, cvRect((display->width - eigImage->width) / 2, (display->height- eigImage->height) / 2, eigImage->width, eigImage->height));
	cvCopy(eigImage, display);
	unROI(display);

	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 0.75, 0.75, 0, 1, CV_AA);
	char text[255];
	point = cvPoint(display->width - 110, display->height - font.line_type/2);
	sprintf(text,"eigen vectors");
	cvPutText(display, text, point, &font, CV_RGB(128,128,128));

	IplImage *tmp = CV::Rotate90(display, 0);
    point = cvPoint(100, font.line_type);
    sprintf(text,"reconstruction error (normalized)");
	cvPutText(tmp, text, point, &font, CV_RGB(128,128,128));
	IMKILL(display);
	display = CV::Rotate90(tmp, 1);
    IMKILL(tmp);

    IMKILL(eigImage);
    accumulator = 0;
    sprintf(text,"Comp#: EigenValue Cumulative");
    cvPutText(display, text, cvPoint(display->width/2, font.line_type), &font, CV_RGB(128,128,128));
    FOR(i, dim-2)
    {
        float eigVal = eigenValues.at<float>(i);
        float prevEigVal = i>0 ? eigenValues.at<float>(i-1) : 0;
        float nextEigVal = i < max(eigenValues.cols,eigenValues.rows)-1 ? eigenValues.at<float>(i+1) : 0;
        int y = font.line_type*(i+2);
        if(y > display->height) continue;
        if(!std::isnan(eigVal))
        {
            accumulator += eigVal / maxEigVal;
            sprintf(text,"e%d: %.2f %.1f%%", i+1, eigVal, accumulator*100);
        }
        else if(i > 0 && i < dim-3 && !std::isnan(prevEigVal) && !std::isnan(nextEigVal))
        {
            float middleEigVal = (prevEigVal + nextEigVal)/2;
            float newAccumulator = accumulator + nextEigVal/maxEigVal;
            sprintf(text,"e%d: %.2f %.1f%%", i+1, middleEigVal, newAccumulator*100);
        }
        else sprintf(text, "e%d: Numeric Error, %.1f%%", i+1, accumulator);
        cvPutText(display, text, cvPoint(display->width/2, y), &font, CV_RGB(128,128,128));
    }
    return display;
}

