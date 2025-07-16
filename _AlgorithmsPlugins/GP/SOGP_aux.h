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
void printScalar(double value, FILE *fp,const char *name,bool ascii);
void readScalar(double &value,FILE *fp,const char *name,bool ascii);
void printRV(RowVector rv,FILE *fp,const char *name=NULL,bool ascii=false);
void readRV(RowVector &rv,FILE *fp,const char *name=NULL,bool ascii=false);
void printCV(ColumnVector cv,FILE *fp,const char *name=NULL,bool ascii=false);
void readCV(ColumnVector &cv,FILE *fp,const char *name=NULL,bool ascii=false);
void printMatrix(Matrix m,FILE *fp,const char *name=NULL,bool ascii=false);
void readMatrix(Matrix &m,FILE *fp,const char *name=NULL,bool ascii=false);
//--------------------------------------------------------

//Kernels
//Known Kernels..This could be done better..Reflection?
enum KERNEL{kerRBF,kerPOL, kerPOLY, kerSIG};

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
    virtual SOGPKernel& operator= (const SOGPKernel &k) {
        if (this != &k) {
            m_type = k.m_type;
        }
        return *this;
    }
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
    double getA(){return A;}
    RowVector getWidths(){return widths;}
    virtual RBFKernel& operator= (const RBFKernel &k) {
        if (this != &k) {
            m_type = k.m_type;
            widths = k.widths;
            A = k.A;
        }
        return *this;
    }
    virtual SOGPKernel& operator=(const SOGPKernel& k)
    {
        if (this != &k){
            m_type = k.m_type;
            const RBFKernel *rbf = dynamic_cast<const RBFKernel*>(&k);
            if(rbf)
            {
                widths = rbf->widths;
                A = rbf->A;
            }
        }
        return *this;
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
    POLKernel& operator= (const POLKernel &k) {
        if (this != &k) {
            m_type = k.m_type;
            scales = k.scales;
        }
        return *this;
    }
    virtual SOGPKernel& operator=(const SOGPKernel& k)
    {
        if (this != &k){
            m_type = k.m_type;
            const POLKernel *pol = dynamic_cast<const POLKernel*>(&k);
            if(pol)
            {
                scales = pol->scales;
            }
        }
        return *this;
    }
    RowVector getScales(){return scales;}
private:
    RowVector scales;
    void init(double s){
        RowVector foo(1);foo(1)=s;init(foo);
    }
    void init(RowVector s){
        m_type=kerPOL;scales=s;
    }
};

class POLYKernel: public SOGPKernel{
public:
    virtual ~POLYKernel(){}
    double kernel(const ColumnVector &a, const ColumnVector &b);
    POLYKernel(){
        init(1);
    }
    POLYKernel(int s, double o){
        init(s);
    }
    void printTo(FILE *fp,bool ascii=false){
        printScalar(degree,fp,"degree",ascii);
        printScalar(offset,fp,"offset",ascii);
    }
    void readFrom(FILE *fp,bool ascii=false){
        double value;
        readScalar(value,fp,"scales",ascii); degree = value;
        readScalar(offset,fp,"scales",ascii);
    }
    POLYKernel& operator= (const POLYKernel &k) {
        if (this != &k) {
            m_type = k.m_type;
            degree = k.degree;
            offset = k.offset;
        }
        return *this;
    }
    virtual SOGPKernel& operator=(const SOGPKernel& k)
    {
        if (this != &k){
            m_type = k.m_type;
            const POLYKernel *pol = dynamic_cast<const POLYKernel*>(&k);
            if(pol)
            {
                degree = pol->degree;
                offset = pol->offset;
            }
        }
        return *this;
    }
    int getDegree(){return degree;}
    double getOffset(){return offset;}
private:
    int degree;
    double offset;
    void init(int d=1, double o=0.){
        degree = d;
        offset = o;
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
    ~SOGPParams()
    {
        //delete m_kernel;
    }
    void setKernel(SOGPKernel *kernel){
        if(m_kernel == kernel) return;
        delete m_kernel;
        m_kernel = kernel;
    }
    SOGPParams& operator=(const SOGPParams& k)
    {
        if (this != &k){
            capacity=k.capacity;
            s20 = k.s20;
            delete m_kernel;
            switch(k.m_kernel->m_type){
            //And this
            case kerRBF: m_kernel=new RBFKernel(*((RBFKernel *)k.m_kernel));break;
            case kerPOL: m_kernel=new POLKernel(*((POLKernel *)k.m_kernel));break;
            }
        }
        return *this;
    }

private:
    void setDefs(){
        capacity=0;
        s20=0.1;
        m_kernel=new RBFKernel;
    }
};

#endif
