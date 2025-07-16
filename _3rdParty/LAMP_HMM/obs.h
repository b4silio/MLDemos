#ifndef OBS_H
#define OBS_H
/* ************************************************************************ *
 * ************************************************************************ *

   File: obs.h
   The class CObs defines an observation class and subclasses for HMMs

  * ************************************************************************ *

   Authors: Daniel DeMenthon & Marc Vuilleumier
   Date:  2-18-99

 * ************************************************************************ *

   Modification Log:


 * ************************************************************************ *
   Log for new ideas:
 * ************************************************************************ *
			   Language and Media Processing
			   Center for Automation Research
			   University of Maryland
			   College Park, MD  20742
 * ************************************************************************ *
 * ************************************************************************ */

//===============================================================================

#include <assert.h>
class CObs{

public:

#if 1
	virtual CObs&  operator=(CObs&) = 0;
#endif
#if 1
	virtual CObs& operator+=(CObs& obs) = 0;
	virtual CObs& operator+=(const double k) = 0;
	virtual CObs& operator-=(CObs& obs) = 0;
#endif
	virtual CObs& operator=(const double k) = 0;
	virtual CObs& operator*=(const double k) = 0;
	virtual CObs& operator/=(const double k) = 0;
	virtual ~CObs(){};
	virtual int GetDim(void) = 0;
	virtual int GetInt(int index) = 0;
	virtual void SetInt(int val, int index) = 0;
	virtual double GetDouble(int index) = 0;
	virtual void SetDouble(double val, int index) = 0;
	virtual void SquareCoords(void) = 0;
	virtual void Print(std::ostream&) = 0;
	virtual void ReadFrom(std::ifstream&) = 0;
	virtual void WriteBinaryTo(std::ostream &sequenceFile) = 0;
	virtual void ReadBinaryFrom(std::ifstream &sequenceFile) = 0;
	virtual void ReadHeader(std::ifstream&) = 0;
	virtual void PrintHeader(std::ostream&) = 0;

	virtual CObs* Clone(void) = 0;
	virtual void SetComponents(double val) = 0;
	virtual CObs** AllocateVector(long nbRows, long nbCols) = 0;
	virtual CObs** AllocateVector(long nbObs) = 0;

	virtual CObs*** AllocateOverlappingArray(CObs** obsVect, long nbRows, long nbCols) = 0;
	virtual void DeleteArrayData(CObs** list, long nbObs) = 0;

	virtual void MapValueToComponents(int value) = 0;
	virtual double SquaredDistanceTo(CObs *obs) = 0;
};

//===============================================================================

class CIntObs: public CObs{

public:
	CIntObs(){};
	CIntObs(CObs& obs){
		CIntObs* obs2 = (CIntObs*)(&obs);
		mObs = obs2->Get();};

	CObs&  operator=(CObs& obs){
		CIntObs* obs2 = (CIntObs*)(&obs); mObs = obs2->Get(); return *this;};

	CObs& operator=(const double k){mObs = int(k); return *this;};
	CObs& operator+=(CObs& obs){
		CIntObs* obs2 = (CIntObs*)(&obs); mObs += obs2->Get(); return *this;};
	CObs& operator+=(const double k){mObs = int(mObs+k); return *this;};
	CObs& operator-=(CObs& obs){
		CIntObs* obs2 = (CIntObs*)(&obs); mObs -= obs2->Get(); return *this;};
	CObs& operator*=(const double k){mObs = int(mObs*k); return *this;};
	CObs& operator*=(CIntObs& obs){mObs =  mObs * obs.Get(); return *this;};
	CObs& operator/=(const double k){mObs = int(mObs/k); return *this;};
	void SquareCoords(void){int obsVal = mObs * mObs; mObs = obsVal;};

	CObs* Clone(void){CIntObs* obs = new CIntObs;
					  obs->Set(mObs);
									return (CObs*)obs;};

	int GetDim(void){return 1;};// one dimension
	inline int Get(void){return mObs;};
	inline void Set(int val){mObs = val;};
	inline int GetInt(int index){return mObs;};
	inline void SetInt(int val, int index){mObs = val;};
	inline double GetDouble(int index){return mObs;};
	inline void SetDouble(double val, int index){mObs = int(val);};
	inline void SetComponents(double val){mObs = int(val);};
	void Print(std::ostream &sequenceFile){sequenceFile << mObs << " ";};
	void ReadFrom(std::ifstream &sequenceFile){sequenceFile >> mObs;};
	void ReadBinaryFrom(std::ifstream &sequenceFile){mObs = sequenceFile.get();};
	void WriteBinaryTo(std::ostream &sequenceFile){sequenceFile.put(char(mObs));};
	void ReadHeader(std::ifstream &inFile){
		char magicID[3]; inFile >> magicID; assert(strcmp(magicID, "P5")==0);};
	void PrintHeader(std::ostream &outFile){outFile << "P5" << std::endl;};

	CObs** AllocateVector(long nbRows, long nbCols){
		CIntObs** obsVect = new CIntObs*[nbRows*nbCols];
		assert(obsVect != NULL);
		for(int i=0;i<nbRows*nbCols;i++){
			obsVect[i] = new CIntObs;
			assert(obsVect[i] != NULL);
		}
		return (CObs**)obsVect;};
	CObs** AllocateVector(long nbObs){
		CIntObs** obsVect = new CIntObs*[nbObs];
		for(int i=0;i<nbObs;i++){
			obsVect[i] = new CIntObs;
			assert(obsVect[i] != NULL);
		}
		return ( CObs**)obsVect;};

	CObs*** AllocateOverlappingArray(CObs** obsVect, long nbRows, long nbCols){
		CIntObs*** obsArray = new CIntObs**[nbRows];// array of pointers
		assert(obsArray != NULL);
		CIntObs** obsVect2 = (CIntObs**)obsVect;
		for(int i=0;i<nbRows;i++){
			obsArray[i] = obsVect2 + i * nbCols;
		}
		return (CObs***)obsArray;};

	void DeleteArrayData(CObs** list, long nbObs){// Nothing much to do
		for(long i=0;i<nbObs;i++) delete list[i];};
	void MapValueToComponents(int value){mObs = (77 * value) % 256;};

	double SquaredDistanceTo(CObs *obs){
		CIntObs* obs2 = (CIntObs*)(obs);
		double dist = (mObs - obs2->Get()) * (mObs - obs2->Get());
		return dist;
	}

private:
	int mObs;
};

//===============================================================================

class CVectorObs: public CObs{

public:
	CVectorObs(int dim){mDimension = dim; mVect = SetVector(dim);};
	CVectorObs(CObs& obs){
		CVectorObs* obs2 = (CVectorObs*)(&obs);
		mDimension = obs2->GetDim();
		mVect = SetVector(mDimension);
		for(int i=1;i<=mDimension;i++) mVect[i]= obs2->Get(i);};

	~CVectorObs(void){ delete[] mVect;};

	CObs&  operator=(CObs& obs){
		CVectorObs* obs2 = (CVectorObs*)(&obs);
		int vDim = obs2->GetDim();
		if ((mVect != NULL) && (mDimension != vDim)){
			delete [] mVect;
			mVect = SetVector(vDim);
		}
		else if ((mVect == NULL) && (vDim != 0)){
			mVect = SetVector(vDim);
		}
		mDimension = vDim;
		for(int i=1;i<=mDimension;i++) mVect[i]= obs2->Get(i);
		return *this;};

	CObs& operator=(const double k){
		for(int i=1;i<=mDimension;i++)mVect[i] = k; return *this;};

	CObs& operator+=(CObs& obs){
		CVectorObs* obs2 = (CVectorObs*)(&obs);
		for(int i=1;i<=mDimension;i++)mVect[i] += obs2->Get(i); return *this;};

	CObs& operator+=(const double k){
		for(int i=1;i<=mDimension;i++)mVect[i] += k; return *this;};

	CObs& operator-=(CObs& obs){
		CVectorObs* obs2 = (CVectorObs*)(&obs);
		for(int i=1;i<=mDimension;i++)mVect[i] -= obs2->Get(i); return *this;};

	CObs& operator*=(const double k){
		for(int i=1;i<=mDimension;i++)mVect[i] *= k; return *this;};

	CObs& operator*=(CVectorObs& obs){
		for(int i=1;i<=mDimension;i++)mVect[i] *= obs.Get(i); return *this;};

	CObs& operator/=(const double k){
		for(int i=1;i<=mDimension;i++)mVect[i] /= k; return *this;};

	void SquareCoords(void){
		double obsVal;
		for(int i=1;i<=mDimension;i++){
			obsVal = mVect[i] * mVect[i];
			mVect[i] = obsVal;
		}
	}

	CObs* Clone(void){CVectorObs* obs = new CVectorObs(mDimension);
					  for(int i=1;i<=mDimension;i++) obs->Set(mVect[i], i);
									return (CObs*)obs;};

	int GetDim(void){return mDimension;};
	inline double* GetVect(void){return mVect;};
	inline double Get(int index){return mVect[index];};
	inline void Set(double val, int index){mVect[index] = val;};
	inline int GetInt(int index){return int(mVect[index]);};
	inline void SetInt(int val, int index){mVect[index] = val;};
	inline double GetDouble(int index){return mVect[index];};
	inline void SetDouble(double val, int index){mVect[index] = val;};
	inline void SetComponents(double val){for(int i=1;i<=mDimension;i++) mVect[i] = val;};
	void Print(std::ostream &sequenceFile)
	{for(int i=1;i<=mDimension;i++) sequenceFile << mVect[i] << " ";};
	void ReadFrom(std::ifstream &sequenceFile)
	{for(int i=1;i<=mDimension;i++) sequenceFile >> mVect[i];};

	void WriteBinaryTo(std::ostream &sequenceFile)
	{for(int i=1;i<=mDimension;i++) sequenceFile.put(char(mVect[i]));};
	void ReadBinaryFrom(std::ifstream &sequenceFile)
	{for(int i=1;i<=mDimension;i++) mVect[i] = sequenceFile.get();};
	void ReadHeader(std::ifstream &inFile){
		char magicID[3]; inFile >> magicID; assert(strcmp(magicID, "P6")==0);};
	void PrintHeader(std::ostream &outFile){outFile << "P6" << std::endl;};

	CObs** AllocateVector(long nbRows, long nbCols){
		CVectorObs** obsVect = new CVectorObs*[nbRows*nbCols];
		assert(obsVect != NULL);
		for(int i=0;i<nbRows*nbCols;i++){
			obsVect[i] = new CVectorObs(mDimension);
			assert(obsVect[i] != NULL);
		}
		return (CObs**)obsVect;};

	CObs** AllocateVector(long nbObs){
		CVectorObs** obsVect = new CVectorObs*[nbObs];
		for(int i=0;i<nbObs;i++){
			obsVect[i] = new CVectorObs(mDimension);
			assert(obsVect[i] != NULL);
		}
		return (CObs**)obsVect;};

	CObs*** AllocateOverlappingArray(CObs** obsVect, long nbRows, long nbCols){
		CVectorObs*** obsArray = new CVectorObs**[nbRows];// array of pointers
		assert(obsArray != NULL);
		CVectorObs** obsVect2 = (CVectorObs**)obsVect;
		for(int i=0;i<nbRows;i++){
			obsArray[i] = obsVect2 + i * nbCols;
		}
		return (CObs***)obsArray;};

	void DeleteArrayData(CObs** list, long nbObs){
		for(long i=0;i<nbObs;i++) delete list[i];};

	void MapValueToComponents(int value){
		int jump = 77 * value;
		for(int i=1;i<=mDimension;i++) mVect[i] = (jump + (i-1)*85) % 256;};

	double SquaredDistanceTo(CObs *obs){
		CVectorObs* obs2 = (CVectorObs*)(obs);
		double dist = 0.0; double delta;
		for(int i=1;i<=mDimension;i++){
			delta = mVect[i] - obs2->Get(i);
			dist += delta * delta;
		}
		return dist;
	}
private:
	int mDimension;
	double *mVect;
};

//===============================================================================

template <class obsType>
class CFlexibleObs: public CObs{

public:
	CFlexibleObs(int dim){mDimension = dim; mVect = new obsType[mDimension+1];};
	CFlexibleObs(CObs& obs){
		CFlexibleObs* obs2 = (CFlexibleObs*)(&obs);
		mDimension = obs2->GetDim();
		mVect = new obsType[mDimension+1];
		for(int i=1;i<=mDimension;i++) mVect[i]= obs2->Get(i);};

	~CFlexibleObs(void){delete[] mVect;};

	CObs&  operator=(CObs& obs){
		CFlexibleObs* obs2 = (CFlexibleObs*)(&obs);
		int vDim = obs2->GetDim();
		if ((mVect != NULL) && (mDimension != vDim)){
			delete [] mVect;
			mVect = new obsType[vDim+1];
		}
		else if ((mVect == NULL) && (vDim != 0)){
			mVect = new obsType[vDim+1];
		}
		mDimension = vDim;
		for(int i=1;i<=mDimension;i++) mVect[i]= obs2->Get(i);
		return *this;};

	CObs& operator=(const double k){
		for(int i=1;i<=mDimension;i++)mVect[i] = k; return *this;};

	CObs& operator+=(CObs& obs){
		CFlexibleObs* obs2 = (CFlexibleObs*)(&obs);
		for(int i=1;i<=mDimension;i++)mVect[i] += obs2->Get(i); return *this;};

	CObs& operator+=(const double k){
		for(int i=1;i<=mDimension;i++)mVect[i] += k; return *this;};

	CObs& operator-=(CObs& obs){
		CFlexibleObs* obs2 = (CFlexibleObs*)(&obs);
		for(int i=1;i<=mDimension;i++)mVect[i] -= obs2->Get(i); return *this;};

	CObs& operator*=(const double k){
		for(int i=1;i<=mDimension;i++)mVect[i] *= k; return *this;};

	CObs& operator*=(CFlexibleObs& obs){
		for(int i=1;i<=mDimension;i++)mVect[i] *= obs.Get(i); return *this;};

	CObs& operator/=(const double k){
		for(int i=1;i<=mDimension;i++)mVect[i] /= k; return *this;};

	void SquareCoords(void){
		double obsVal;
		for(int i=1;i<=mDimension;i++){
			obsVal = mVect[i] * mVect[i];
			mVect[i] = obsVal;
		}
	}

	CObs* Clone(void){CFlexibleObs* obs = new CFlexibleObs<obsType>(mDimension);
					  for(int i=1;i<=mDimension;i++) obs->Set(mVect[i], i);
									return (CObs*)obs;};

	inline obsType* GetVect(void){return mVect;};
	inline obsType Get(int index){return mVect[index];};
	int GetDim(void){return mDimension;};
	inline void Set(double val, int index){mVect[index] = (obsType)val;};
	inline void SetComponents(double val){for(int i=1;i<=mDimension;i++) mVect[i] = (obsType)val;};
	inline int GetInt(int index){return int(mVect[index]);};
	inline void SetInt(int val, int index){mVect[index] = (obsType)val;};
	inline double GetDouble(int index){return (obsType)(mVect[index]);};
	inline void SetDouble(double val, int index){mVect[index] = (obsType)val;};
	void Print(std::ostream &sequenceFile)
	{for(int i=1;i<=mDimension;i++) sequenceFile << mVect[i] << " ";};
	void ReadFrom(std::ifstream &sequenceFile)
	{for(int i=1;i<=mDimension;i++) sequenceFile >> mVect[i];};

	void WriteBinaryTo(std::ostream &sequenceFile)
	{for(int i=1;i<=mDimension;i++) sequenceFile.put(char(mVect[i]));};
	void ReadBinaryFrom(std::ifstream &sequenceFile)
	{for(int i=1;i<=mDimension;i++) mVect[i] = sequenceFile.get();};
	void ReadHeader(std::ifstream &inFile){
		char magicID[3]; inFile >> magicID; assert(strcmp(magicID, "P6")==0);};
	void PrintHeader(std::ostream &outFile){outFile << "P6" << std::endl;};

	CObs** AllocateVector(long nbRows, long nbCols){
		CFlexibleObs** obsVect = new CFlexibleObs*[nbRows*nbCols];
		assert(obsVect != NULL);
		for(int i=0;i<nbRows*nbCols;i++){
			obsVect[i] = new CFlexibleObs(mDimension);
			assert(obsVect[i] != NULL);
		}
		return (CObs**)obsVect;};
	CObs** AllocateVector(long nbObs){
		CFlexibleObs** obsVect = new CFlexibleObs*[nbObs];
		for(int i=0;i<nbObs;i++){
			obsVect[i] = new CFlexibleObs(mDimension);
			assert(obsVect[i] != NULL);
		}
		return (CObs**)obsVect;};

	CObs*** AllocateOverlappingArray(CObs** obsVect, long nbRows, long nbCols){
		CFlexibleObs*** obsArray = new CFlexibleObs**[nbRows];// array of pointers
		assert(obsArray != NULL);
		CFlexibleObs** obsVect2 = (CFlexibleObs**)obsVect;
		for(int i=0;i<nbRows;i++){
			obsArray[i] = obsVect2 + i * nbCols;
			//obsArray[i] = &(obsVect2[i * nbCols]);
		}
		return (CObs***)obsArray;};

	void DeleteArrayData(CObs** list, long nbObs){
		for(long i=0;i<nbObs;i++) delete list[i];};

	void MapValueToComponents(int value){
		int jump = 77 * value;
		for(int i=1;i<=mDimension;i++) mVect[i] = (jump + (i-1)*85) % 256;};

	double SquaredDistanceTo(CObs *obs){
		CFlexibleObs* obs2 = (CFlexibleObs*)(obs);
		double dist = 0.0; double delta;
		for(int i=1;i<=mDimension;i++){
			delta = mVect[i] - obs2->Get(i);
			dist += delta * delta;
		}
		return dist;
	}

private:
	int mDimension;
	obsType *mVect;
};

//===============================================================================
//===============================================================================
//===============================================================================

#endif
