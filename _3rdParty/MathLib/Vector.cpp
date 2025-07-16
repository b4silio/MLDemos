/*
 * Copyright (C) 2010 Learning Algorithms and Systems Laboratory, EPFL, Switzerland
 * Author: Eric Sauser
 * email:   eric.sauser@a3.epf.ch
 * website: lasa.epfl.ch
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#include "Vector.h"
#include "Matrix.h"
#ifdef USE_MATHLIB_NAMESPACE
using namespace MathLib;
#endif

REALTYPE Vector::undef = R_ZERO;

Matrix Vector::MultTranspose(const Vector & vec){
    Matrix res;
    MultTranspose(vec,res);
    return res;
}

Matrix& Vector::MultTranspose(const Vector & vec, Matrix& result){
  result.Resize(row,vec.row);
  for(unsigned int i=0;i<row;i++){
    for(unsigned int j=0;j<vec.row;j++){
      result._[i*vec.row+j] = _[i]*vec._[j];
    }
  }
  return result;
}

void Vector::Print() const{
    Print("");
}
void Vector::Print(string name) const{
  PRINT_BEGIN(cout);

  //cout << "Vector " <<row<<" "<<name<<endl;;
  for (unsigned int i = 0; i < row; i++){

    cout <<"| ";
    cout.width(PRINT_WIDTH);
    cout<< _[i] ;
    cout<<" |"<<endl;

  }

  PRINT_END(cout);
}

/*
std::ostream& operator<<(std::ostream& out, const Vector& a){
  PRINT_BEGIN(out);

  for (unsigned int i = 0; i < a.Size(); i++){
    out.width(PRINT_WIDTH);
    out<< a.AtNoCheck(i) ;

  }

  PRINT_END(out);
  return out;
}
*/
