#include "Matrix.h"

#ifdef USE_MATHLIB_NAMESPACE
using namespace MathLib;
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;

int   Matrix::bInverseOk = TRUE;

string Matrix::RemoveSpaces(string s){
  basic_string <char>::size_type pos;
  if(s.size()==0)
    return s;

  string ms = s;
  pos = ms.find_first_of(" \t\n");
  while(pos==0){
    ms = ms.substr(1);
    pos = ms.find_first_of(" \t\n");
  }
  pos = ms.find_last_of(" \t\n");
  while(pos==ms.size()-1){
    ms = ms.substr(0,ms.size()-1);
    pos = ms.find_last_of(" \t\n");
  }
  return ms;
}


bool Matrix::Load(const char* filename){
    vector<string> data;
    ifstream file;
    file.open(filename);
    char buf[4096];
    if(file.is_open()){
        file.getline(buf,4096);
        int row = 0;    
        while(!file.eof()){
            data.push_back(RemoveSpaces(string(buf)));
            if(data[row].length()>0){
                row++;
            }else{
                break;    
            }
            file.getline(buf,4096);    
        }
        file.close();
        if(row <= 0){
            Resize(0,0,false);
            return false;
        }
        double val;       
        stringstream ss(data[0]);
        int col = 0;
        while(!ss.eof()){
            ss >> val; col++;  
        }
        if(col <= 0){
            Resize(0,0,false);
            return false;            
        }
        Resize(row,col,false); Zero();
        cout << "File: "<<filename<<" - Matrix size : ("<<row<<","<<col<<")"<<endl;
        
        bool bRet = true;
        for(int i=0;i<row;i++){
            stringstream ss(data[i]);
            //double val;
            int cnt = 0;
            while((!ss.eof())&&(cnt<col)){
                ss >> _[i*column+cnt]; cnt++;
            }    
            if(cnt!=col) bRet = false;           
        }
        return bRet;
    }
    return false;       
}

bool Matrix::Save(const char* filename, unsigned int precision){
    ofstream file;
    file.open(filename);
    if(file.is_open()){
        file.precision(precision);
        int row = RowSize();
        int col = ColumnSize();
        for(int i=0;i<row;i++){
            for(int j=0;j<col;j++){
                file << _[i*column+j]<< " ";
            }            
            file << endl;
        }
        file.close();
        return true;
    }else{
        return false;    
    }
}

/// Load a matrix from filename
bool Matrix::LoadBinary(const char* filename){
    ifstream file;
    file.open(filename,ios::in|ios::binary);
    if(file.is_open()){
        unsigned int size[2];
        file.read((char*)size,2*sizeof(unsigned int));
        Resize(size[0],size[1],false);
        file.read((char*)_,row*column*sizeof(REALTYPE));
        file.close();
        return true;
    }else{
        return false;    
    }
    return false;
}
/// Save a matrix to filename
bool Matrix::SaveBinary(const char* filename){
    ofstream file;
    file.open(filename,ios::out|ios::binary);
    if(file.is_open()){
        unsigned int size[2];
        size[0] = row;
        size[1] = column;
        file.write((char*)size,2*sizeof(unsigned int));
        file.write((char*)_,row*column*sizeof(REALTYPE));
        file.close();
        return true;
    }else{
        return false;    
    }
}

bool MathLib::TestClassMatrix(){
    cout << "TESTING CLASS MATRIX"<<endl;
    {
        Matrix A;
        Matrix B(A);
        Matrix C(8,8);
        C.Random();
        Matrix D(C);
        Matrix E(C.Array(),8,8);
        bool res = (A==B) && (C==D) && (E==C);
        cout <<"- Constructor: "<<(res?"<ok>":"<fail>")<<endl;
    }
    {
        Matrix N0(8,8);
        Matrix  A(8,8);
        Matrix N1(8,8);
        /*
        A.Identity();
        A+=2;
        //A/=10;
        //A.Resize(4,12);
        A.SetRow(3,0);
        A.SetColumn(4,1);
        Vector v;
        A.SetRow(A.GetColumn(3),1);
        A.Print();
        A.GetColumns(6,9).SwapColumn(0,1).Print();
        A.Identity();
        A-=1.0;
        Matrix B(3,3);
        B.Identity();
        B+=1.0;
        (A^=B).Print();
        */
        A.Resize(6,10);
        A.Random();
        Matrix B(12,6);
        B.Random();
        Matrix C;
        A.Mult(B,C);
        Matrix D;
        A.Print();
        //B.STranspose();
        //B.Print();
        A.MultTranspose2(B,D);
        A.Identity();
        A+=1.0;
        B.Identity(); 
        B+=2.0;
        A.HCat(B,C);
        A.Resize(6,6);
        A.Random();
        A.MultTranspose2(A,B);
        //B.InverseSymmetric(C);
        //(C*B).Print();
        A = B;
        //A.Inverse(B);
        //(A*B).Print();
        A.Tridiagonalize(B,C);
        A.Print();
        B.Print();
        C.Print();
        
        Matrix IA;
        A.InverseSymmetric(IA);
        Matrix EVec;
        Vector EVal;
        A.EigenValuesDecomposition2(EVal,EVec);
        D.Diag(EVal);
        ((EVec*D*EVec.Transpose())-A).Print();
        A.Print();
        //bool res = C==D;//A.Sum() == 2.0*A.RowSize()*A.ColumnSize();
        bool res = (N0.Sum()==0) && (N1.Sum()==0);
        cout <<"- Basic Functions: "<<(res?"<ok>":"<fail>")<<endl;
    }
    {
        Matrix A(8,8);
        SharedMatrix B(A);
        bool res = (A==B);
        cout <<"- Shared Matrix Constructor: "<<(res?"<ok>":"<fail>")<<endl;
    }
    return false;
    
}
