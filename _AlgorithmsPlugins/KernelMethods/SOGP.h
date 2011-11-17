/*Sparse Online Gaussian Process Regression
Math by Csato and Opper
Coding by Dan Grollman (dang@cs.brown.edu)

Brown University
Copyright 2008

Equation and page references are to Csato's Thesis.

TODO:  Allow for params to change
       How to do this without recalculating entire C and K?
Auto adapt parameters?
*/

#ifndef __SOGP_H__
#define __SOGP_H__

#include "SOGP_aux.h"

class SOGP{
 public:
  //Constructors
  SOGP(){//defaults
    SOGPParams params;
    setParams(params);
  }
  SOGP(SOGPParams params){
    setParams(params);
  }
  //1D RBF case
  SOGP(double w,double s20, int cap=0){
    RBFKernel kern(w);
    SOGPParams params(&kern);
    params.s20=s20;
    params.capacity=cap;
    setParams(params);
  };

  //Add data to the SOGP
  void add(const ColumnVector& in,const ColumnVector& out);
  //Predict output and sigma or confidence (0-100)
  ReturnMatrix predict(const ColumnVector& in,double &sigconf,bool conf=false);
  //Don't care about sigma
  ReturnMatrix predict(const ColumnVector& in){
    double foo;
    return predict(in,foo);
  }

  //These two just wrap the single-data versions
  void addM(const Matrix& in, const Matrix& out);
  ReturnMatrix predictM(const Matrix& in, ColumnVector &sigconf,bool conf=false);
  ReturnMatrix predictM(const Matrix& in){
    ColumnVector foo;
    return predictM(in,foo);
  }

  //Return the log probability of this pair under the GP
  double log_prob(const ColumnVector& in, const ColumnVector& out);

  //File IO
  bool save(const char *fn,bool ascii=false){
    FILE *fp = fopen(fn,"w");
    int ret=printTo(fp,ascii);
    fclose(fp);
    return ret;
  }
  bool printTo(FILE *fp,bool ascii=false);
  bool load(const char *fn,bool ascii=false){
    FILE *fp = fopen(fn,"r");
    int ret=readFrom(fp,ascii);
    fclose(fp);
    return ret;
  }
  bool readFrom(FILE *fp,bool ascii=false);

  //Accessors...could use cleaning
  int size(){
    return current_size;
  }
  double BVloc(int ind,int dim){
    if(ind<BV.Ncols() && dim < BV.Nrows())
      return BV(dim+1,ind+1);
    else
      return 0;//Not quite correct
  }
  double alpha_acc(int ind,int dim){
    if(ind<alpha.Nrows() && dim < alpha.Ncols())
      return alpha(ind+1,dim+1);
    else
      return 0;
  }
  void change_capacity(int cap){
    m_params.capacity=cap;
  }

  int dim(){return BV.Nrows();}

 private: 
  int current_size;  //how many points do I have
  Matrix alpha;      //Alpha and C are the parameters of the GP
                     //Alpha is NxDout
  Matrix C;            
  Matrix Q;          //Inverse Gram Matrix.  C and Q are NxN
  Matrix BV;         //The Basis Vectors
                     //BV is DinxN
  //parameters
  SOGPParams m_params;
  
  //Removal function
  void delete_bv(int loc);

  //Set the parameters.  Maybe public for reset?
  void setParams(SOGPParams params){
    m_params=params;
    current_size=0;
  }
};


#endif
