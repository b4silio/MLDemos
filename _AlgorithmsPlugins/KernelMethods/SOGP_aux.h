/*Aux stuff for SOGP*/

#ifndef __SOGP_AUX__
#define __SOGP_AUX__

//Newmat Library and print extension
#define WANT_MATH
#include "newmat11/newmatap.h"
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef WIN32
#pragma warning(disable:4996)
#endif

//--------------------------------------------------------
//These could be in a library?  In newmat?  Reads should autodetect?
void printRV(RowVector rv,FILE *fp,const char *name=NULL,bool ascii=false);
void readRV(RowVector &rv,FILE *fp,const char *name=NULL,bool ascii=false);
void printCV(ColumnVector cv,FILE *fp,const char *name=NULL,bool ascii=false);
void readCV(ColumnVector &cv,FILE *fp,const char *name=NULL,bool ascii=false);
void printMatrix(Matrix m,FILE *fp,const char *name=NULL,bool ascii=false);
void readMatrix(Matrix &m,FILE *fp,const char *name=NULL,bool ascii=false);
//--------------------------------------------------------

//Kernels
//Known Kernels..This could be done better..Reflection?
enum KERNEL{kerRBF,kerPOL};

//A kernel is the function, and it's parameters
class SOGPKernel{
public:
    virtual ~SOGPKernel(){}
    virtual double kernel(const ColumnVector& a, const ColumnVector& b)=0;
    virtual ReturnMatrix kernelM(const ColumnVector& in, const Matrix &BV);
    virtual double kstar(const ColumnVector& in);
    virtual double kstar();
    virtual void printTo(FILE *fp,bool ascii=false){
        printf("Kernel Writer %d not written\n",m_type);
    }
    virtual void readFrom(FILE *fp,bool ascii=false){
        printf("Kernel Reader %d not written\n",m_type);
    }
    KERNEL m_type;
};

class RBFKernel: public SOGPKernel{
public:
    virtual ~RBFKernel(){}
    double kernel(const ColumnVector &a, const ColumnVector &b);
    void printTo(FILE *fp,bool ascii = false){
        fprintf(fp,"A %lf\n",A);printRV(widths,fp,"widths",ascii);
    }
    void readFrom(FILE *fp,bool ascii=false){
        fscanf(fp,"A %lf\n",&A);readRV(widths,fp,"widths",ascii);
    }
    RBFKernel(){
        init(.1);
    }
    RBFKernel(double w){
        init(w);
    }
    RBFKernel(RowVector w){
        init(w);
    }
    //Must call this explicitly...shouldn't need to
    void setA(double nA){
        A=nA;
    }
private:
    double A;//Should likely never change, but just in case
    RowVector widths;//Stored as 1/w
    void init(double w){
        RowVector foo(1);foo(1)=w;init(foo);
    }
    void init(RowVector w){//This is the base case
        m_type=kerRBF;widths=w;A=1;
        for(int i=1;i<=widths.Ncols();i++)//Invert the widths
            widths(i)=1.0/widths(i);
    }
};

class POLKernel: public SOGPKernel{
public:
    virtual ~POLKernel(){}
    double kernel(const ColumnVector &a, const ColumnVector &b);
    POLKernel(){
        init(1);
    }
    POLKernel(double s){
        init(s);
    }
    POLKernel(RowVector s){
        init(s);
    }
    void printTo(FILE *fp,bool ascii=false){
        printRV(scales,fp,"scales",ascii);
    }
    void readFrom(FILE *fp,bool ascii=false){
        readRV(scales,fp,"scales",ascii);
    }
private:
    RowVector scales;
    void init(double s){
        RowVector foo(1);foo(1)=s;init(foo);
    }
    void init(RowVector s){
        m_type=kerPOL;scales=s;
    }
};

//SOGP Parameters
class SOGPParams{
public:
    //should params be private and 'set'?
    int capacity;
    double s20;  //Should be a vector?
    SOGPKernel *m_kernel;//Cannot edit kernel once made.

    void printTo(FILE *fp,bool ascii=false){
        fprintf(fp,"capacity %d, s20 %lf\n",capacity,s20);
        fprintf(fp,"kernel %d ",m_kernel->m_type);
        m_kernel->printTo(fp,ascii);
    };
    void readFrom(FILE *fp,bool ascii=false){
        fscanf(fp,"capacity %d, s20 %lf\n",&capacity,&s20);
        int temp;
        fscanf(fp,"kernel %d ",&temp);
        switch(temp){
        //This should be better?
        case kerRBF: m_kernel=new RBFKernel();break;
        case kerPOL: m_kernel=new POLKernel();break;
        default: printf("SOGPParams readFrom: Unknown Kernel! %d\n",temp);
        }
        m_kernel->readFrom(fp,ascii);
    }
    SOGPParams(){
        setDefs();
    }
    SOGPParams(SOGPKernel *kern){
        setDefs();
        delete m_kernel;
        switch(kern->m_type){
        //And this
        case kerRBF: m_kernel=new RBFKernel(*((RBFKernel *)kern));break;
        case kerPOL: m_kernel=new POLKernel(*((POLKernel *)kern));break;
        }
    }
private:
    void setDefs(){
        capacity=0;
        s20=0.1;
        m_kernel=new RBFKernel;
    }
};

#endif
