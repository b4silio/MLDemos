#include "pcaprojection.h"
#include "widget.h"
#include <QDebug>

ProjectionPCA::ProjectionPCA()
{
}

PCA ProjectionPCA::compressPCA(const Mat& pcaset, int maxComponents, const Mat& testset, Mat& compressed)
{
    PCA pca(pcaset, // pass the data
            Mat(), // we do not have a pre-computed mean vector,
                   // so let the PCA engine to compute it
            CV_PCA_DATA_AS_ROW, // indicate that the vectors
                                // are stored as matrix rows
                                // (use CV_PCA_DATA_AS_COL if the vectors are
                                // the matrix columns)
            maxComponents // specify, how many principal components to retain
            );
    // if there is no test data, just return the computed basis, ready-to-use
    if( !testset.data )
        return pca;
    CV_Assert( testset.cols == pcaset.cols );

    compressed.create(testset.rows, maxComponents, testset.type());

    Mat reconstructed;
    for( int i = 0; i < testset.rows; i++ )
    {
        Mat vec = testset.row(i), coeffs = compressed.row(i);
        // compress the vector, the result will be stored
        // in the i-th row of the output matrix
        pca.project(vec, coeffs);
        // and then reconstruct it
        pca.backProject(coeffs, reconstructed);
        // and measure the error
        //printf("");
    }
    return pca;
}

IplImage *ProjectionPCA::DrawEigenvals(Mat eigVal)
{
    IplImage *eigImage = cvCreateImage(cvSize(440,440),8,3);
    cvSet(eigImage, CV_RGB(255,255,255));

    int dim = eigVal.rows;
    float maxEigVal = 0;
    FOR(i, dim) if(eigVal.at<float>(i) == eigVal.at<float>(i)) maxEigVal += eigVal.at<float>(i);
    float accumulator = 0;
    maxEigVal = max(1.f,maxEigVal);

    cvDrawLine(eigImage, cvPoint(0, eigImage->height-1), cvPoint(eigImage->width, eigImage->height-1), CV_RGB(180,180,180));
    cvDrawLine(eigImage, cvPoint(0,0), cvPoint(0, eigImage->height), CV_RGB(180,180,180));
    CvPoint point = cvPoint(0,0);
    FOR(i, dim)
    {
        float eigval = eigVal.at<float>(i);
        if(eigval == eigval)
        {
            CvPoint point2 = cvPoint(i * eigImage->width / dim, eigImage->height - (int)(eigval / maxEigVal * eigImage->height));
            cvDrawLine(eigImage, point, point2, CV_RGB(0,0,0));
            accumulator += eigval / maxEigVal;
            printf("%d\t(%.2f) %.1f%%\n", i+1, eigval, accumulator*100);
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
    point = cvPoint(tmp->width - 110, font.line_type);
    sprintf(text,"data percent");
    cvPutText(tmp, text, point, &font, CV_RGB(128,128,128));
    IMKILL(display);
    display = CV::Rotate90(tmp, 1);
    IMKILL(tmp);

    IMKILL(eigImage);
    accumulator = 0;
    sprintf(text,"comp");
    cvPutText(display, text, cvPoint(display->width/2, font.line_type+3), &font, CV_RGB(128,128,128));
    sprintf(text,"cumul%%");
    cvPutText(display, text, cvPoint(display->width/2 + 40, font.line_type+3), &font, CV_RGB(128,128,128));
    sprintf(text,"eigenvalue");
    cvPutText(display, text, cvPoint(display->width/2 + 100, font.line_type+3), &font, CV_RGB(128,128,128));
    FOR(i, dim)
    {
        int y = (font.line_type)*(i+2)+3;
        if(y > display->height) continue;
        float eigval = eigVal.at<float>(i);
        if(eigval == eigval)
        {
            accumulator += eigval / maxEigVal;
            sprintf(text,"e%d:", i+1);
            cvPutText(display, text, cvPoint(display->width/2, y), &font, CV_RGB(128,128,128));
            sprintf(text,"%.1f%%", accumulator*100);
            cvPutText(display, text, cvPoint(display->width/2 + 40, y), &font, CV_RGB(128,128,128));
            sprintf(text,"%.1f", eigval);
            cvPutText(display, text, cvPoint(display->width/2 + 100, y), &font, CV_RGB(128,128,128));
        }
    }
    return display;
}

void ProjectionPCA::Train(std::vector<fvec> samples, int pcaCount)
{
    if(!samples.size() || !samples[0].size()) return;

    int count = samples.size();
    int dim = samples[0].size();
    pcaCount = min(dim, pcaCount);
    Mat pcaData = Mat::zeros(count, dim, CV_32F);
    FOR(i, count)
    {
        FOR(d,dim)
        {
            pcaData.at<float>(i,d) = samples[i][d];
        }
    }
    Mat output;
    pca = compressPCA(pcaData, pcaCount, pcaData, output);
    qDebug() << "PCA size: " << pca.eigenvalues.rows << pca.eigenvalues.cols;
    FOR(i, pca.eigenvalues.rows)
    {
        FOR(j, pca.eigenvalues.cols)
        {
            qDebug() << i << pca.eigenvalues.at<float>(i);
        }
    }
    this->samples = vector<fvec>(count);
	vector<bool> bNan(pcaCount,false);
	int nanCnt = 0;
	FOR(d, pcaCount)
	{
		if(bNan[d] = pca.eigenvalues.at<float>(d) != pca.eigenvalues.at<float>(d)) nanCnt++;
	}
	FOR(i, count)
	{
		this->samples[i].resize(pcaCount-nanCnt);
		int D = 0;
		FOR(d, pcaCount)
		{
			if(bNan[d]) continue;
			this->samples[i][D] = output.at<float>(i,d);
			D++;
		}
	}
}

QLabel* ProjectionPCA::EigenValues()
{
    IplImage *eigValsImg = DrawEigenvals(pca.eigenvalues);
    QLabel* eigenValueLabel = new QLabel();
    eigenValueLabel->setPixmap(QNamedWindow::toPixmap(eigValsImg));
    IMKILL(eigValsImg);
    return eigenValueLabel;
}
