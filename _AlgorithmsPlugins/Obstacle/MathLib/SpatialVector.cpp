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

#include "SpatialVector.h"
#ifdef USE_MATHLIB_NAMESPACE
using namespace MathLib;
#endif

#include <iostream>
using namespace std;
/*
std::ostream & operator<<(std::ostream& out, const SpatialVector & a){
  PRINT_BEGIN(out);


    out.width(PRINT_WIDTH); out<< a.cwx();
    out.width(PRINT_WIDTH); out<< a.cwy();
    out.width(PRINT_WIDTH); out<< a.cwz();
    out.width(PRINT_WIDTH); out<< a.cx();
    out.width(PRINT_WIDTH); out<< a.cy();
    out.width(PRINT_WIDTH); out<< a.cz();

  PRINT_END(out);
  return out;
}
*/
void SpatialVector::Print() const {
    Print("");
}
void SpatialVector::Print(string name) const {
  PRINT_BEGIN(cout);


  cout << "SpatialVector " <<name<<endl;;
  for (unsigned int i = 0; i < 3; i++){
    cout <<"| ";
    cout.width(PRINT_WIDTH);
    cout<< mAngular._[i] ;
    cout<<" |"<<endl;
  }
  for (unsigned int i = 0; i < 3; i++){
    cout <<"| ";
    cout.width(PRINT_WIDTH);
    cout<< mLinear._[i] ;
    cout<<" |"<<endl;
  }
  PRINT_END(cout);
}
