#include <iostream>
#include <QErrorMessage>

#include "projectorCVO.h"
#include "CVOLearner.h"

namespace {

fvec meanVector( const std::vector<fvec>& data )
{
    int dataSize = int(data.size());
    fvec mean;
    for( size_t i = 0; i < data.at(0).size(); ++i ) { // Init vec
        mean.push_back(0.0);
    }
    // Calc mean for each dimension
    for( size_t i = 0; i < dataSize; ++i ) { // Loop over data points
        for( size_t j = 0; j < mean.size(); ++j ) { // for each dimension
            mean[j] += data.at(i).at(j);
        }
    }
    // Divide by number of elements
    for( size_t i = 0; i < mean.size(); ++i ) {
        mean[i] /= dataSize;
    }
    return mean;
}

fvec standardDev( const std::vector<fvec>& data, const fvec& mean )
{
    fvec stddev;
    for( size_t i = 0; i < mean.size(); ++i ) { // Init vec
        stddev.push_back(0.0);
    }
    size_t dataSize = data.size();
    for( size_t i = 0; i < dataSize; ++i ) { // Loop over data points
        for( size_t j = 0; j < mean.size(); ++j ) { // for each dimension
            float center = data[i][j] - mean[j];
            stddev[j] += (center * center) / dataSize;
        }
    }
    // Sqrt
    for( size_t j = 0; j < mean.size(); ++j ) { // for each dimension
        stddev[j] = sqrt(stddev.at(j));
    }
    return stddev;
}

/**
 * @brief Normalize data, substract mean divide by standard deviation
 * @param data
 */
void normalize( std::vector<fvec>& data )
{
    fvec mean = meanVector(data);
    fvec stddev = standardDev(data, mean);

     size_t dataSize = data.size();
     for( size_t i = 0; i < dataSize; ++i ) { // Loop over data points
         for( size_t j = 0; j < mean.size(); ++j ) { // for each dimension
             float center = data[i][j] - mean[j];
             data[i][j] = center / stddev[j];
         }
     }
}

} // end anonymous namespace

ProjectorCVO::ProjectorCVO()
    : m_learner( new CVOLearner() )
    , m_simClass(-1)
    , m_dissimClass(-1)
    , m_allOthersDissimilar(false)
    , m_normalize(false)
{
}

ProjectorCVO::~ProjectorCVO()
{
    delete m_learner;
}

void ProjectorCVO::setClasses( int simClass, int dissimClass, bool allOthers )
{
    m_simClass = simClass;
    m_dissimClass = dissimClass;
    m_allOthersDissimilar = allOthers;
}

std::pair<ProjectorCVO::classMap, bool> ProjectorCVO::checkAndSortInput( const ivec& labels )
{
    classMap indexMap;

    for( size_t i = 0; i < labels.size(); ++i ) {
        indexMap.insert(std::make_pair(labels.at(i), i));
    }

    std::vector<int> keys;
    // iterate through unique keys add them in a collection
    for( mapIter it = indexMap.begin(), end = indexMap.end();
         it != end; it = indexMap.upper_bound(it->first)) {
        keys.push_back(it->first);
    }

    if( keys.size() < 2 ) {
        qErrnoWarning("ProjectorCVO::checkAndSortInput need at least 2 classes");
        return std::make_pair(indexMap, false);
    }

    if( std::find(keys.begin(), keys.end(), m_simClass) == keys.end() ) { // Class not found in vector
        qErrnoWarning("ProjectorCVO::checkAndSortInput similar class not found in dataset");
        return std::make_pair(indexMap, false);
    }

    if( !m_allOthersDissimilar ) {
        if( std::find(keys.begin(), keys.end(), m_dissimClass) == keys.end() || m_simClass == m_dissimClass ) {
            qErrnoWarning("ProjectorCVO::checkAndSortInput dissimilar class not found in dataset");
            return std::make_pair(indexMap, false);
        }
    }
    return std::make_pair(indexMap, true);
}

ProjectorCVO::fvecVecPair ProjectorCVO::splitDataset( const fvecVec& samples, const classMap& indexMap )
{
    fvecVec simData;
    fvecVec dissimData;

    // Iterate over unique keys
    for( cmapIter it = indexMap.begin(), end = indexMap.end();
         it != end; it = indexMap.upper_bound(it->first)) {
        int key = (*it).first;
        std::pair<cmapIter, cmapIter> keyRange = indexMap.equal_range(key);
        // Iterate over values
        for( cmapIter v_it = keyRange.first;  v_it != keyRange.second; ++v_it ) {
            if( key == m_simClass ) {
                simData.push_back(samples.at((*v_it).second));
            }
            else if( !m_allOthersDissimilar ) {
                if( key == m_dissimClass ) {
                    dissimData.push_back(samples.at((*v_it).second));
                }
            }
            else if( m_allOthersDissimilar ) { // all others are dissimilar
                dissimData.push_back(samples.at((*v_it).second));
            }
            else {
                ;
            }
        }
    }
    return std::make_pair(simData, dissimData);
}

void ProjectorCVO::Train( fvecVec samples, ivec labels )
{
    source = samples;
    std::pair<ProjectorCVO::classMap, bool> res = checkAndSortInput(labels);
    if( !res.second ) {
        m_learner->setIsValid(false);
        return;
    }

    if( m_normalize ) {
        normalize(samples);
    }

    // Ok input valid
    fvecVecPair data = splitDataset(samples, res.first);
    m_learner->train(data.first, data.second);

    // Project new
    projected.clear();
    projected.reserve(samples.size());
    for( size_t i = 0; i < samples.size(); ++i ) {
        fvec tmp = Project(samples.at(i));
        projected.push_back(tmp);
    }
}

fvec ProjectorCVO::Project( const fvec& sample )
{
    if( m_learner->isValid() )
        return m_learner->project(sample);
    else
        return sample;
}

void ProjectorCVO::setAlpha( float alpha )
{
    m_learner->setAlpha(alpha);
}

void ProjectorCVO::setSteps( int steps )
{
    m_learner->setSteps(steps);
}

void ProjectorCVO::setMethod( int method )
{
    m_learner->setMethod(method);
}

ProjectorCVO::fvecVec ProjectorCVO::matrixCoeff()
{
    // Construct vector<vector<float> from Eigen Matrix
    fvecVec res;
    if( !m_learner->isValid() )
        return res;

    CVOLearner::MatrixXXf coeff = m_learner->coeff();
    int rows = coeff.rows();
    int cols = coeff.cols();
    res.reserve(rows);
    for( size_t i = 0; i < rows; ++i ) {
        fvec row;
        row.reserve(cols);
        for( size_t j = 0; j < cols; ++j ) {
            row.push_back( coeff(i,j) );
        }
        res.push_back(row);
    }
    return res;
}

