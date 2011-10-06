#include "pcaprojection.h"

PCAProjection::PCAProjection()
{
}

PCA PCAProjection::compressPCA(const Mat& pcaset, int maxComponents, const Mat& testset, Mat& compressed)
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

void PCAProjection::Train(std::vector<fvec> samples, int pcaCount)
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
    PCA pca = compressPCA(pcaData, pcaCount, pcaData, output);
    this->samples = vector<fvec>(count);
    FOR(i, count)
    {
        this->samples[i].resize(pcaCount);
        FOR(d, pcaCount)
        {
            this->samples[i][d] = output.at<float>(i,d);
        }
    }
}
