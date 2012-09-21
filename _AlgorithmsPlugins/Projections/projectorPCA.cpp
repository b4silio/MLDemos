#include "projectorPCA.h"
#include "widget.h"
#include <QPainter>
#include <QDebug>

ProjectorPCA::ProjectorPCA()
{}

PCA ProjectorPCA::compressPCA(const Mat& pcaset, int maxComponents, const Mat& testset, Mat& compressed)
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

void ProjectorPCA::DrawEigenvals(QPainter &painter)
{
    int w=painter.window().width();
    int h=painter.window().height();
    int pad = 5;

    Mat& eigVal = pca.eigenvalues;
    int dim = eigVal.rows;
    float maxEigVal = 0;
#ifdef OPENCV21 // legacy
    FOR(i, dim) if(eigVal.at<float>(i,1) == eigVal.at<float>(i,1)) maxEigVal += eigVal.at<float>(i,1);
#else // OPENCV22+
    FOR(i, dim) if(eigVal.at<float>(i) == eigVal.at<float>(i)) maxEigVal += eigVal.at<float>(i);
#endif
    maxEigVal = max(1.f,maxEigVal);
    float maxAccumulator = 0;
#ifdef OPENCV21 // legacy
    FOR(i, dim) if(eigVal.at<float>(i,1) == eigVal.at<float>(i,1)) maxAccumulator += eigVal.at<float>(i,1) / maxEigVal;
#else // OPENCV22+
    FOR(i, dim) if(eigVal.at<float>(i) == eigVal.at<float>(i)) maxAccumulator += eigVal.at<float>(i) / maxEigVal;
#endif
    float accumulator = 0;

    painter.setPen(Qt::black);
    painter.drawLine(QPointF(pad, h-2*pad), QPointF(w-pad, h-2*pad));
    painter.drawLine(QPointF(pad, pad), QPointF(pad, h-2*pad));
    painter.setRenderHint(QPainter::Antialiasing);
    QPointF point(pad,pad);
    painter.setPen(Qt::red);
    FOR(i, dim)
    {
#ifdef OPENCV21 // legacy
        float eigval = eigVal.at<float>(i,1);
#else // OPENCV22+
        float eigval = eigVal.at<float>(i);
#endif
        if(eigval == eigval)
        {
            accumulator += eigval / maxEigVal;
            qDebug() << "accumulator: " << accumulator << accumulator/maxAccumulator;
            QPointF point2 = QPointF(dim==1 ? w/2 : i * (w-2*pad) / (dim-1) + pad+(!i?1:0), (int)(accumulator/maxAccumulator * (h-2*pad)));
            painter.drawLine(point, point2);
            point = point2;
        }
        else
        {
            point.setX(dim==1 ? w/2 : i * (w-2*pad) / (dim-1) + pad+(!i?1:0));
        }
    }
    //painter.drawLine(point, QPoint(w-2*pad, h-2*pad));
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(0,0,w,2*pad,Qt::AlignCenter, "reconstruction error");
    int step = 1;
    while((dim/step > 8)) step++;
    for(int i=0; i<dim; i+=step)
    {
        int x = dim==1? w/2 : i * (w-2*pad) / (dim-1) + pad+(!i?1:0);
        if(i==dim-1) x -= 4;
        //        int x = dim==1? w/2 : i*(w-2*pad)/(dim-1);
        painter.drawText(x - 4, h-1, QString("e%1").arg(i+1));
    }
}

void ProjectorPCA::TrainPCA(std::vector<fvec> samples, int pcaCount)
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
    projected = vector<fvec>(count);
    vector<bool> bNan(pcaCount,false);
    int nanCnt = 0;
    FOR(d, pcaCount)
    {
#ifdef OPENCV21 // legacy
        if(bNan[d] = pca.eigenvalues.at<float>(d,1) != pca.eigenvalues.at<float>(d,1)) nanCnt++;
#else // OPENCV22+
        if(bNan[d] = pca.eigenvalues.at<float>(d) != pca.eigenvalues.at<float>(d)) nanCnt++;
#endif
    }
    FOR(i, count)
    {
        projected[i].resize(pcaCount-nanCnt);
        int D = 0;
        FOR(d, pcaCount)
        {
            if(bNan[d]) continue;
            projected[i][D] = output.at<float>(i,d);
            D++;
        }
    }
}

void ProjectorPCA::Train(std::vector< fvec > samples, ivec labels)
{
    projected.clear();
    source.clear();
    if(!samples.size()) return;
    source = samples;
    dim = samples[0].size();
    if(!dim) return;
    if(stopIndex >= (int)dim) stopIndex = -1;
    if(startIndex && startIndex >= (int)dim) startIndex = dim-1;
    if(stopIndex != -1 && startIndex > stopIndex) stopIndex = startIndex;
    int pcaCount = (stopIndex != -1) ? stopIndex+1 : min((int)dim, (int)samples.size()-1);
    TrainPCA(samples, pcaCount);
    if(startIndex)
    {
        vector<fvec> newProjected(projected.size());
        if(!projected.size()) return;
        int pDim = projected[0].size();
        FOR(i, projected.size())
        {
            newProjected[i].resize(pDim - startIndex);
            FOR(d, pDim - startIndex) newProjected[i][d] = projected[i][d + startIndex];
        }
    }
}

fvec ProjectorPCA::Project(const fvec &sample)
{
    return sample;
}

fvec ProjectorPCA::GetEigenValues()
{
    fvec values(dim);
    FOR(i, dim)
    {
        float eigval = pca.eigenvalues.at<float>(i,0);
        values[i] = eigval;
    }
    return values;
}

vector<fvec> ProjectorPCA::GetEigenVectors()
{
    int cols = pca.eigenvectors.cols;
    int rows = pca.eigenvectors.rows;
    vector<fvec> eigenVectors(rows);
    FOR(i, rows)
    {
        eigenVectors[i].resize(cols);
        FOR(j, cols)
        {
            eigenVectors[i][j] = pca.eigenvectors.at<float>(i, j);
        }
    }
    return eigenVectors;
}
