#ifndef PROJECTORCVO_H
#define PROJECTORCVO_H

#include <projector.h>

class CVOLearner; // forward declaration

class ProjectorCVO : public Projector
{
    typedef std::multimap<int, size_t> classMap;
    typedef classMap::iterator mapIter;
    typedef classMap::const_iterator cmapIter;
    typedef std::vector<fvec> fvecVec;
    typedef std::pair<fvecVec, fvecVec> fvecVecPair;

public:
    ProjectorCVO();
    ~ProjectorCVO();

    void setClasses( int simClass, int dissimClass, bool allOthers );
    void setAlpha( float alpha );
    void setSteps( int steps );
    void setMethod( int method );
    void setNormalizeData( bool v ) { m_normalize = true; }

    void Train( fvecVec samples, ivec labels );
    fvec Project( const fvec& sample );
    const char* GetInfoString() { return "CVX Opt Metric Learner"; }
    fvecVec matrixCoeff();

private:
    /**
     * @brief Verify input, number of classes and options
     * @param labels
     * @return Multimap of key: class, value: index of each sample associated with the class, bool success
     */
    std::pair<classMap, bool> checkAndSortInput( const ivec& labels );
    /**
     * @brief Split dataset in 2 classes
     * @param samples input
     * @param indexMap multimap of sorted classes
     * @return pair of vector<vector<float>> for similar and dissimilar points
     */
    fvecVecPair splitDataset( const fvecVec& samples, const classMap& indexMap );

private:
    int m_simClass;
    int m_dissimClass;
    bool m_allOthersDissimilar;
    bool m_normalize;
    CVOLearner* m_learner;
};

#endif // PROJECTORCVO_H
