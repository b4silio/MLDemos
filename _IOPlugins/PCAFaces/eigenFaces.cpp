#include <public.h>
#include "basicMath.h"
#include "basicOpenCV.h"
#include "eigenFaces.h"

EigenFaces::EigenFaces()
{
	eigenValues = NULL;
	eigenVectors = NULL;
	avgImage = NULL;
	mapImage = NULL;
	bUseColor = false;
}

EigenFaces::~EigenFaces()
{
	if(eigenValues) cvReleaseMat(&eigenValues);
    if(eigenVectors)
    {
        FOR(i, dim) IMKILL(eigenVectors[i]);
    }
	KILL(eigenVectors);
	IMKILL(avgImage);
	IMKILL(mapImage);
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

void EigenFaces::Learn(std::vector<IplImage *> faces, std::vector<int> classes, std::vector<bool> isTrainingData, bool bColor)
{
	if(!faces.size() || !faces[0]) return;

	if(eigenVectors)
	{
		FOR(i, dim) IMKILL(eigenVectors[i]);
		delete [] eigenVectors;
		eigenVectors = NULL;
	}

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

	CvTermCriteria calcLimit;
	CvSize res = cvGetSize(faces[0]);
	if(bUseColor) res.width *= 3;
	trainCnt = this->classes.size();
	
	IplImage **allFaces = new IplImage *[this->classes.size()];
	int cnt = 0;
	FOR(i, faces.size())
	{
		//if(classes[i] == 0) continue;
		if(!bUseColor)
		{
			if(faces[i]->nChannels == 3)
			{
				allFaces[cnt] = cvCreateImage(res,8,1);
				cvCvtColor(faces[i], allFaces[cnt], CV_BGR2GRAY);
				cvEqualizeHist(allFaces[cnt], allFaces[cnt]);
			}
			else allFaces[cnt] = cvCloneImage(faces[i]);
		}
		else
		{
			allFaces[cnt] = cvCreateImage(res, 8, 1);
			if(faces[i]->nChannels == 3)
			{
				FOR(j, res.width*res.height) allFaces[cnt]->imageData[j] = faces[i]->imageData[j];
			}
			else
			{
				allFaces[cnt] = cvCreateImage(res, 8, 1);
				FOR(j, res.width*res.height) allFaces[cnt]->imageData[j] = faces[i]->imageData[j/3];
			}
		}
		cnt++;
	}

	FOR(i, isTraining.size()) if(!isTraining[i]) trainCnt--;
	dim = trainCnt - 1;
	IplImage **faceArray = new IplImage *[trainCnt];
	cnt = 0;
	FOR(i, isTraining.size()) if(isTraining[i])
	{
		faceArray[cnt++] = cvCloneImage(allFaces[i]);
	}
	eigenVectors = new IplImage *[dim];
	FOR(i, dim) eigenVectors[i] = cvCreateImage(res, IPL_DEPTH_32F, 1);

	if(eigenValues) cvReleaseMat(&eigenValues);
	eigenValues = cvCreateMat(1, dim, CV_32FC1);

	IMKILL(avgImage);
	avgImage = cvCreateImage(res, IPL_DEPTH_32F, 1);

	calcLimit = cvTermCriteria(CV_TERMCRIT_ITER, dim, 0.0001);

	cvCalcEigenObjects(trainCnt, (void *)faceArray, (void *)eigenVectors, CV_EIGOBJ_NO_CALLBACK, 0, 0, &calcLimit, avgImage, eigenValues->data.fl);

	// we compute the projections for recognition purposes
	FOR(i, projections.size()) delete [] projections[i];
	projections.clear();
	FOR(i, this->classes.size())
	{
		float *proj = new float[dim];
		cvEigenDecomposite(allFaces[i], dim, eigenVectors, 0, 0, avgImage, proj);
		projections.push_back(proj);
	}
	FOR(i, trainCnt) cvReleaseImage(&faceArray[i]);
	KILL(faceArray);
	FOR(i, this->classes.size()) cvReleaseImage(&allFaces[i]);
	KILL(allFaces);
}

void EigenFaces::Recognize(IplImage *face)
{
	if(!face) return;
	float *projected = new float[dim];
	cvEigenDecomposite(face, dim, eigenVectors, 0, 0, avgImage, projected);
	int nearest = FindNearestNeighbor(projected);
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
			if(bMahalanobis) dist /= eigenValues->data.fl[i];
		}

		if(dist < minDist)
		{
			minDist = dist;
			nearest = train;
		}
	}

	return nearest;
}

void eigen_on_mouse( int event, int x, int y, int flags, void* param )
{
	IplImage **eigenVectors = (IplImage **)(((int *)param)[0]);
	IplImage *avgImage = (IplImage *)(((int *)param)[1]);
	int dim = (*(int *)(((int *)param)[2]));
	float *maxes = (float *)(((int *)param)[3]);
	std::vector<float *> *projections = (std::vector<float *> *)(((int *)param)[4]);
	float size = maxes[4];
	int e1 = ((int *)param)[5];
	int e2 = ((int *)param)[6];

	IplImage *acc = cvCreateImage(cvGetSize(eigenVectors[0]), IPL_DEPTH_32F, 1);
	cvCopy(avgImage, acc);
	float *coords = new float[dim];
	FOR(i, dim) coords[i] = 0;

	float c1 = x / size * maxes[2] + maxes[0];
	float c2 = y / size * maxes[3] + maxes[1];

	// we find the closest point among the training samples
	/*
	int closest = 0;
	float dist = FLT_MAX;
	FOR(i, projections->size())
	{
		float d = sqrtf(powf(c1-projections->at(i)[e1],2) + powf(c2-projections->at(i)[e2],2));
		if(d < dist)
		{
			dist = d;
			closest = i;
		}
	}
	FOR(i, dim) coords[i] = projections->at(closest)[i];
	*/
	coords[e1] = c1;
	coords[e2] = c2;

	FOR(i, dim-2) // dim -2 because for some reasons the last two eigenvalues are often fubared
	{
		cvAddWeighted(eigenVectors[i], coords[i], acc, 1, 0, acc);
	}

	IplImage *newEigen = cvCreateImage(cvGetSize(eigenVectors[0]), 8, 1);
	cvCvtScale(acc, newEigen, 1);
	bool bUseColor = newEigen->width != newEigen->height;

	IplImage *display = cvCreateImage(cvSize(128,128), 8, bUseColor ? 3 : 1);
	if(!bUseColor) cvResize(newEigen, display, CV_INTER_CUBIC);
	else
	{
		IplImage *newEigen3 = cvCreateImage(cvSize(newEigen->width/3, newEigen->height), 8, 3);
		FOR(i, newEigen->width*newEigen->height) newEigen3->imageData[i] = newEigen->imageData[i];
		cvResize(newEigen3, display, CV_INTER_CUBIC);
		IMKILL(newEigen3);
	}
	cvNamedWindow("eigen face");
	cvShowImage("eigen face", display);
	IMKILL(newEigen);
	IMKILL(display);

}

int eigparams[7];

std::vector<IplImage *> EigenFaces::GetEigenVectorsImages()
{
	std::vector<IplImage *> result;
	IplImage *acc = cvCreateImage(cvGetSize(eigenVectors[0]), IPL_DEPTH_32F, 1);
	FOR(i, dim-2) // dim -2 because for some reasons the last two eigenvalues are often fubared
	{
		cvSet(acc, cvScalar(0));
		//cvCopy(avgImage, acc);
		cvAddWeighted(eigenVectors[i], 1, acc, 1, 0, acc);
		cvNormalize(acc, acc, 255,0,CV_MINMAX);

		IplImage *newEigen = cvCreateImage(cvGetSize(eigenVectors[0]), 8, 1);
		cvCvtScale(acc, newEigen, 1);
		bool bUseColor = newEigen->width != newEigen->height;

		IplImage *res = cvCreateImage(cvSize(128,128), 8, bUseColor ? 3 : 1);
		if(!bUseColor) cvResize(newEigen, res, CV_INTER_CUBIC);
		else
		{
			IplImage *newEigen3 = cvCreateImage(cvSize(newEigen->width/3, newEigen->height), 8, 3);
			FOR(i, newEigen->width*newEigen->height) newEigen3->imageData[i] = newEigen->imageData[i];
			cvResize(newEigen3, res, CV_INTER_CUBIC);
			IMKILL(newEigen3);
		}
		result.push_back(res);
	}
	IMKILL(acc);
	return result;
}

IplImage *EigenFaces::DrawEigenVals()
{
    IplImage *eigImage = cvCreateImage(cvSize(440,440),8,3);
	cvSet(eigImage, CV_RGB(255,255,255));

	float *eigVal = eigenValues->data.fl;
	float maxEigVal = 0;
    FOR(i, dim-2) if(eigVal[i] == eigVal[i]) maxEigVal += eigVal[i];
	float accumulator = 0;
    maxEigVal = max(1.f,maxEigVal);

	cvDrawLine(eigImage, cvPoint(0, eigImage->height-1), cvPoint(eigImage->width, eigImage->height-1), CV_RGB(180,180,180));
	cvDrawLine(eigImage, cvPoint(0,0), cvPoint(0, eigImage->height), CV_RGB(180,180,180));
	CvPoint point = cvPoint(0,0);
	printf("eigenval\tdata\n");
	FOR(i, dim-2) // dim -2 because for some reasons the last two eigenvalues are often fubared
	{
        if(eigVal[i] == eigVal[i])
        {
            CvPoint point2 = cvPoint(i * eigImage->width / dim, eigImage->height - (int)(eigVal[i] / maxEigVal * eigImage->height));
            //cvDrawCircle(eigImage, point, 1, CV_RGB(0,0,0), -1, CV_AA);
            cvDrawLine(eigImage, point, point2, CV_RGB(0,0,0));
            accumulator += eigVal[i] / maxEigVal;
            printf("%d\t(%.2f) %.1f%%\n", i+1, eigVal[i], accumulator*100);
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
        int y = font.line_type*(i+2);
        if(y > display->height) continue;
        if(eigVal[i] == eigVal[i])
        {
            accumulator += eigVal[i] / maxEigVal;
            sprintf(text,"e%d: %.2f %.1f%%", i+1, eigVal[i], accumulator*100);
        }
        else if(i > 0 && i < dim-3 && eigVal[i-1] == eigVal[i-1] && eigVal[i+1] == eigVal[i+1])
        {
            float middleEigVal = (eigVal[i-1] + eigVal[i+1])/2;
            float newAccumulator = accumulator + eigVal[i+1]/maxEigVal;
            sprintf(text,"e%d: %.2f %.1f%%", i+1, middleEigVal, newAccumulator*100);
        }
        else sprintf(text, "e%d: Numeric Error, %.1f%%", i+1, accumulator);
        cvPutText(display, text, cvPoint(display->width/2, y), &font, CV_RGB(128,128,128));
    }
    return display;
}

void EigenFaces::Draw(bool bMonochrome, int e1, int e2)
{
	if(!eigenValues || !eigenVectors) return;

	if(e1 >= dim) e1 = dim-1;
	if(e2 >= dim) e2 = dim-1;

	// we find min and max values for each dimension
	float minX, maxX, minY, maxY;
	minX = minY = FLT_MAX;
	maxX = maxY = FLT_MIN;
	float dX, dY;

	FOR(i, trainCnt)
	{
		if(minX > projections[i][e1]) minX = projections[i][e1];
		if(maxX < projections[i][e1]) maxX = projections[i][e1];
		if(minY > projections[i][e2]) minY = projections[i][e2];
		if(maxY < projections[i][e2]) maxY = projections[i][e2];
	}
	dX = maxX - minX;
	dY = maxY - minY;

	int size = 512;
	int edge = 20;
	mapImage = cvCreateImage(cvSize(size,size), 8, 3);
	cvZero(mapImage);
	const int radius = 3;
	
	FOR(i, projections.size())
	{
		cvVec2 v((projections[i][e1] - minX)/dX, (projections[i][e2] - minY)/dY);
		CvPoint point = (v*((float)size-edge*2) + cvVec2((f32)edge, (f32)edge)).to2d();
		if(!isTraining[i])
		{
			cvCircle(mapImage, point, radius, CV_RGB(180,180,180),2,CV_AA);
		}
		cvCircle(mapImage, point, radius,
			bMonochrome ? CV_RGB(255,255,255) : CV::color[classes[i]%CV::colorCnt],
			1, CV_AA);
	}

	float *maxes = new float[5];
	maxes[0] = minX; maxes[1] = minY;
	maxes[2] = dX; maxes[3] = dY;
	maxes[4] = (float)size;
	CvSize *res = new CvSize;
	(*res) = cvSize(size, size);
	eigparams[0] = (intptr_t)((void *)eigenVectors);
	eigparams[1] = (intptr_t)((void *)avgImage);
	eigparams[2] = (intptr_t)((void *)&dim);
	eigparams[3] = (intptr_t)((void *)maxes);
	eigparams[4] = (intptr_t)((void *)&projections);
	eigparams[5] = (intptr_t) e1;
	eigparams[6] = (intptr_t) e2;


	IplImage *legend = cvCreateImage(cvSize(100,100),8,3);
	cvZero(legend);
	cvLine(legend, cvPoint(20, 80), cvPoint(90, 80), CV_RGB(255,255,255));
	cvLine(legend, cvPoint(20,10), cvPoint(20, 80), CV_RGB(255,255,255));
	
	/*
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 0.75, 0.75, 0, 1, CV_AA);
	char text[255];
	sprintf(text, "e%d", e1+1);
	CvPoint point = cvPoint(legend->width-30, legend->height - font.line_type/2);
	cvPutText(legend, text, point, &font, CV_RGB(255,255,255));
	IplImage *tmp = CV::Rotate90(legend, 0);
	point = cvPoint(tmp->width - 35, font.line_type);
	sprintf(text,"e%d", e2+1);
	cvPutText(tmp, text, point, &font, CV_RGB(255,255,255));
	IMKILL(legend);
	legend = CV::Rotate90(tmp, 1);
	IMKILL(tmp);	

	ROI(mapImage, cvRect(mapImage->width-legend->width, 0, legend->width, legend->height));
	cvAddWeighted(mapImage, 1, legend, 0.6, 0, mapImage);
	unROI(mapImage);
	IMKILL(legend);
	*/


	cvNamedWindow("Principal Components");
	cvShowImage("Principal Components", mapImage);
	cvSetMouseCallback("Principal Components", eigen_on_mouse, (void *) &eigparams);
}

