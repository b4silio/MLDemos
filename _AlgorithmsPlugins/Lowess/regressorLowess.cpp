/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "public.h"
#include "regressorLowess.h"
#include "gsl/multifit/gsl_multifit.h"
#include "gsl/sort/gsl_sort.h"
#include "lowessHelpers.h"
#include "gsl/matrix/gsl_matrix.h"
#include <algorithm>
#include <QMessageBox>
#include <QDebug>
#include <assert.h>

//Some convenient debugging macros
#ifdef DEBUG
    #define DEBUG_PRINT_2DFVEC(_debug_vec,_debug_vecName) {\
        QString _debugString;\
        FOR (_debug_i, (_debug_vec).size())\
        {\
            FOR (_debug_j, (_debug_vec)[0].size())\
                    _debugString += QString("%1 ").arg((_debug_vec)[_debug_i][_debug_j],0,'f',3);\
            _debugString += '\n';\
        }\
        qDebug() << (_debug_vecName) << (_debugString);\
    }
#else
    #define DEBUG_PRINT_2DFVEC(_debug_vec,_debug_vecName)
#endif

#ifdef DEBUG
    #define DEBUG_PRINT_FVEC(_debug_vec,_debug_vecName) {\
        QString _debugString;\
        FOR (_debug_i, (_debug_vec).size())\
            _debugString += QString("%1 ").arg((_debug_vec)[_debug_i],0,'f',3);\
        qDebug() << (_debug_vecName) << (_debugString);\
    }
#else
    #define DEBUG_PRINT_FVEC(_debug_vec,_debug_vecName)
#endif

#ifdef DEBUG
    #define DEBUG_PRINT_IARR(_debug_vec,_debug_vecName,_debug_vecLen) {\
        QString _debugString;\
        FOR (_debug_i, (_debug_vecLen))\
            _debugString += QString("%1 ").arg((_debug_vec)[_debug_i],0);\
        qDebug() << (_debug_vecName) << (_debugString);\
    }
#else
    #define DEBUG_PRINT_IARR(_debug_vec,_debug_vecName,_debug_vecLen)
#endif


using namespace std;

RegressorLowess::RegressorLowess()
    : smoothingFac (0.01),
      weightingFunc(kLowessWeightFunc_Tricube),
      fitType      (kLowessFitType_Quadratic),
      normType     (kLowessNormType_None),
      zeroSpread   (true),
      tooFewPoints (true),
      workspace    (NULL),
      X            (NULL),
      y            (NULL),
      cov          (NULL),
      weights      (NULL),
      c            (NULL),
      x            (NULL)
{
}

RegressorLowess::~RegressorLowess()
{
    if (workspace != NULL)
        gsl_multifit_linear_free(workspace);
    workspace = NULL;

    if (X != NULL)
        gsl_matrix_free(X);
    X = NULL;

    if (y != NULL)
        gsl_vector_free(y);
    y = NULL;

    if (cov != NULL)
        gsl_matrix_free(cov);
    cov = NULL;

    if (weights != NULL)
        gsl_vector_free(weights);
    weights = NULL;

    if (c != NULL)
        gsl_vector_free(c);
    c = NULL;

    if (x != NULL)
        gsl_vector_free(x);
    x = NULL;
}

void RegressorLowess::SetParams(double param1, lowessWeightFunc param2,
                                lowessFitType param3, lowessNormType param4)
{
    smoothingFac  = param1;
    weightingFunc = param2;
    fitType       = param3;
    normType      = param4;
}

void RegressorLowess::Train(std::vector< fvec > trainSamples, ivec labels)
{
    if(!trainSamples.size()) return;
    dim = trainSamples[0].size();

    //Copy training data
    samples.clear();
    samples = trainSamples;

    // outputdim is the dimension that we want to estimate, and it might NOT be the last one in the dataset
    if(outputDim != -1 && outputDim < dim-1)
    {
        // we need to swap the current last dimension with the desired output
        FOR(i, samples.size())
        {
            float val = samples[i][dim-1];
            samples[i][dim-1] = samples[i][outputDim];
            samples[i][outputDim] = val;
        }
    }

    //Compute the mean and standard deviation across each predictor dimension
    means. clear();
    stdevs.clear();
    iqrs.  clear();
    FOR(j, dim-1)
    {
        float mean, stdev, iqr;
        fvec tempCopy;
        FOR(i, samples.size())
            tempCopy.push_back(samples[i][j]);

        calcDescriptiveStats(tempCopy, &mean, &stdev, &iqr);
        means. push_back(mean);
        stdevs.push_back(stdev);
        iqrs.  push_back(iqr);
    }

    //Make sure that there is some variance in at least one dimension of the
    //input data. Otherwise the distance is the same to all input points
    //(since they all have the exact same coordinates), and we cannot sort for
    //the numNearestNeighbors closest data points.
    zeroSpread = (*max_element(stdevs.begin(), stdevs.end()) <= 0.0f);
    if (zeroSpread)
    {
        showErrorMsg_zeroSpread();
        return; //we can already stop here
    }


    //Determine number of fit params      v---- constant term
    numFitParams  = (1+fitType)*(dim-1) + 1;
    //In the case of multivariate regression, we also have cross-product terms
    numCrossProds = dim < 3 ? 0 : (dim-1)*(dim-1-1)/2;
    numFitParams += numCrossProds;

    numNearestNeighbors = (int)(smoothingFac * samples.size() + 0.5);
    numNearestNeighbors = MIN(MAX(numNearestNeighbors, 1), samples.size());
    radiusVec.clear();

    //Make sure that the number of fit parameters is not larger than the number
    //of data points used for the fit, i.e. numFitParams <= numNearestNeighbors,
    //otherwise our system has too many degrees of freedom and the SVD won't succeed.
    tooFewPoints = (numFitParams > numNearestNeighbors);
    if (tooFewPoints)
    {
        showErrorMsg_tooFewPoints();
        return; //we can already stop here
    }

    //Initialize various arrays for GSL multifit

    if (workspace != NULL)
        gsl_multifit_linear_free(workspace);
    workspace = gsl_multifit_linear_alloc(numNearestNeighbors, numFitParams);

    if (X != NULL)
        gsl_matrix_free(X);
    X = gsl_matrix_alloc(numNearestNeighbors, numFitParams);

    if (y != NULL)
        gsl_vector_free(y);
    y = gsl_vector_alloc(numNearestNeighbors);

    if (cov != NULL)
        gsl_matrix_free(cov);
    cov = gsl_matrix_alloc(numFitParams, numFitParams);

    if (weights != NULL)
        gsl_vector_free(weights);
    weights = gsl_vector_alloc(numNearestNeighbors);

    if (c != NULL)
        gsl_vector_free(c);
    c = gsl_vector_alloc(numFitParams);

    if (x != NULL)
        gsl_vector_free(x);
    x = gsl_vector_alloc(numFitParams);

    return;
}

fvec RegressorLowess::Test(const fvec &sample)
{
    fvec res;
    res.resize(2,0);

    if (!Ready()) //no regression possible, exit directly
    {
        res[0] = res[1] = 0.0f;
        return res;
    }

    //Compute distance of current sample to training samples
    fvec distances(samples.size());
    calcDistances(sample, distances);

    //Sort the distances to find the nearest neighbors
    //We need to use a stable sort, otherwise points that have identical
    //coordinates except for outputDim may be selected in a random fashion.
    size_t *sortIndexes = new size_t[samples.size()];
    FOR(i, samples.size())
        sortIndexes[i] = i;
    mergesort_perm(&distances[0], sortIndexes, 0, samples.size()-1);
    radius = distances[sortIndexes[numNearestNeighbors-1]];

    //The Tricube and Hann weightings have smooth contact with zero, therefore
    //the farthest datapoint with distance=radius will actually have zero weight!
    //Here we make sure that we have enough other data points at lower distances.
    float minWeight = 0.0f;
    if (weightingFunc != kLowessWeightFunc_Uniform)
    {
        int nextLowerDist = numNearestNeighbors-1;
        while(nextLowerDist >= 0 && distances[sortIndexes[nextLowerDist]] > radius - 1e-6f)
            nextLowerDist--;
        if ((fitType == kLowessFitType_Quadratic && nextLowerDist < 2) ||
            (fitType == kLowessFitType_Linear    && nextLowerDist < 1) ||
             nextLowerDist < 0) //Not enough other data points -->
            minWeight = 1e-6f;  //assign minimum weight to avoid under-defined eq. system
    }

    //Set weights for samples within current regression window
    FOR(i, numNearestNeighbors)
        gsl_vector_set(weights, i, calcWeighting(distances[sortIndexes[i]], radius, minWeight));

    //Copy nearest samples to GSL matrix
    if (fitType == kLowessFitType_Linear)
        FOR(i, numNearestNeighbors)
        {
            gsl_matrix_set(X, i, 0, 1.0); //constant term
            FOR(j, dim-1)
                gsl_matrix_set(X, i, j+1, samples[sortIndexes[i]][j]);
        }
    else //fitType == kLowessFitType_Quadratic
        FOR(i, numNearestNeighbors)
        {
            fvec const &curSamp = samples[sortIndexes[i]]; //local shorthand
            gsl_matrix_set(X, i, 0, 1.0); //constant term
            FOR(j, dim-1)
            {
                gsl_matrix_set(X, i, 2*j+1, curSamp[j]);
                gsl_matrix_set(X, i, 2*j+2, curSamp[j]*curSamp[j]);
            }
            //In the multivariate case, we also add a cross-product term
            //which is used to model interactions between the predictors.
            int matOffset = 2*dim - 1;
            for (int k = dim-2; k > 0; k--)
                for (int l = 0; l < k; l++, matOffset++)
                    gsl_matrix_set(X, i, matOffset, curSamp[k]*curSamp[l]);
            assert(matOffset - (2*dim-1) == numCrossProds);
        }

    //Copy sorted target values to y vector
    FOR(i, numNearestNeighbors)
        gsl_vector_set(y, i, samples[sortIndexes[i]][dim-1]);

    //Run multifit
    double chisq = 0.0;
    gsl_multifit_wlinear(X, weights, y, c, cov, &chisq, workspace);

    //Get regression estimate at current sample location
    if (fitType == kLowessFitType_Linear)
    {
        gsl_vector_set(x, 0, 1.0);
        FOR(j, dim-1)
            gsl_vector_set(x, j+1, sample[j]);
    }
    else //fitType == kLowessFitType_Quadratic
    {
        gsl_vector_set(x, 0, 1.0);
        FOR(j, dim-1)
        {
            gsl_vector_set(x, 2*j+1, sample[j]);
            gsl_vector_set(x, 2*j+2, sample[j]*sample[j]);
        }
        //Add the cross-product term(s) in the multivariate case
        int vecOffset = 2*dim - 1;
        for (int k = dim-2; k > 0; k--)
            for (int l = 0; l < k; l++, vecOffset++)
                gsl_vector_set(x, vecOffset, sample[k]*sample[l]);
        assert(vecOffset - (2*dim-1) == numCrossProds);
    }
    double y_est, y_err;
    gsl_multifit_linear_est(x, c, cov, &y_est, &y_err);

    res[0]  = y_est;  // the regression estimation
    res[1]  = y_err;  // stdev of the estimation
    res[1] *= res[1]; //convert stdev to variance

    //Clean up
    delete[] sortIndexes;
    sortIndexes = NULL;

    return res;
}

void RegressorLowess::StoreLastRadius()
{
    if (!Ready())
        return;

    radiusVec.push_back(radius);
}


const char *RegressorLowess::GetInfoString()
{
    char *text = new char[1024];
    sprintf(text, "Locally Weighted Scatterplot Smoothing\n");

    sprintf(text, "%sSmoothing factor: %.2f\n", text, smoothingFac);

    sprintf(text, "%sFit Type: %s\n", text,
            fitType == kLowessFitType_Linear ? "Linear" : "Quadratic");

    switch(weightingFunc)
    {
    case kLowessWeightFunc_Tricube: sprintf(text, "%sWeighting Function: %s\n", text, "Tricube");     break;
    case kLowessWeightFunc_Hann:    sprintf(text, "%sWeighting Function: %s\n", text, "Hann Window"); break;
    case kLowessWeightFunc_Uniform: sprintf(text, "%sWeighting Function: %s\n", text, "Uniform");     break;
    }

    if (dim >= 3 && normType != kLowessNormType_None) //multivariate regression
        sprintf(text, "%sDimension Normalization: %s\n", text,
                normType == kLowessNormType_StDev ? "Standard Deviation" :
                                                    "IQR");

    return text;
}


void RegressorLowess::calcDistances(const fvec &sample, fvec &distances)
{
    if (dim < 3) //simple regression
        FOR(i, samples.size())
            distances[i] = fabs(samples[i][0] - sample[0]);
    else
    {
        //In the case of multivariate regression, it is sensible to divide
        //each variable by a measure of its spread, otherwise the distance
        //measure is comparing apples with oranges.
        //We first compute the reciprocal of the selected measure of spread, so
        //we can omit a variable from the total distance below if its spread is 0.
        //(Also, multiplication is much faster than division on x86 processors)
        fvec recipSpread;
        if (normType == kLowessNormType_StDev)
            FOR(j, dim-1)
                recipSpread.push_back(stdevs[j] > 0.0f ? 1/stdevs[j] : 0.0f);
        else if (normType == kLowessNormType_IQR)
            FOR(j, dim-1)
                recipSpread.push_back(iqrs  [j] > 0.0f ? 1/iqrs  [j] : 0.0f);
        else //normType == kLowessNormType_None
            recipSpread.resize(dim-1, 1.0f);

        FOR(i, samples.size())
        {
            float distance = 0.0f;
            FOR(j, dim-1)
                distance += pow((samples[i][j] - sample[j]) * recipSpread[j], 2);
            distances[i] = sqrtf(distance);
        }
    }

    return;
}

float RegressorLowess::calcWeighting(float distance, float radius, float minWeight)
{
    //Normalization factors for the Tricube and Uniform weighting functions,
    //calculated relative to the average value of the Hann window.
    float const NORMFAC_TRICUBE = 0.864197530864197f;
    float const NORMFAC_UNIFORM = 0.5f;

    float distWeight = MAX(1.0f, minWeight);
    if (radius <= 0.0f) //All points have equal distance to current estimate position
        return distWeight; //Apply same weighting to all points

    switch(weightingFunc)
    {
    case kLowessWeightFunc_Tricube:
        distance /= radius;
        distWeight  = 1.0f - distance*distance*distance;
        distWeight  = distWeight*distWeight*distWeight;
        distWeight *= NORMFAC_TRICUBE;
        break;
    case kLowessWeightFunc_Hann:
        distWeight  = 0.5f * (1 + cos(M_PI*distance / radius));
        //The other weighting normalization factors were calculated
        //relative the Hann window, so no normalization is required.
        break;
    case kLowessWeightFunc_Uniform:
        distWeight  = 1.0f;
        distWeight *= NORMFAC_UNIFORM;
        break;
    }

    distWeight = MIN(MAX(distWeight, minWeight), 1.0f);
    return distWeight;
}

void RegressorLowess::showErrorMsg_zeroSpread()
{
    QString informativeText = "The spread of the input data is zero "\
            "in all dimensions.";
    QMessageBox msgBox;
    msgBox.setWindowTitle("LOWESS Error"); //ignored on Mac OS X
    msgBox.setText("No spread in input data for LOWESS");
    msgBox.setInformativeText(informativeText);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void RegressorLowess::showErrorMsg_tooFewPoints()
{
    QString informativeText = "The number of fitting variables ("
            + QString::number(numFitParams)
            + ") is greater than the number of data points in the local neighborhood "\
               "used for fitting ("
            + QString::number(numNearestNeighbors)
            + ").\n\nPlease";
    if (numNearestNeighbors == samples.size() &&
        fitType == kLowessFitType_Linear)
        informativeText += " add more data points.";
    else
    {
        if (numNearestNeighbors < samples.size())
            informativeText += ", increase the smoothing factor";
        if (fitType >= kLowessFitType_Quadratic)
            informativeText += ", use local linear fitting";
        informativeText += " or add more data points.";
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("LOWESS Error"); //ignored on Mac OS X
    msgBox.setText("Not enough data points for LOWESS");
    msgBox.setInformativeText(informativeText);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec(); //Display the error message to the user
}
