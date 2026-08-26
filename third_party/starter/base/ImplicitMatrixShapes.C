
#include "ImplicitMatrixShapes.h"
#include "LinearAlgebra.h"


using namespace lux;
using namespace std;

ConstantMatrixVolume::ConstantMatrixVolume( const Matrix& m ) :
   elem(m)
   {}

const Matrix ConstantMatrixVolume::eval( const Vector& P ) const
{
   return elem;
}




AddMatrixVolume::AddMatrixVolume( Volume<Matrix>* m1, Volume<Matrix>* m2 ) :
  elem1(m1),
  elem2(m2)
  {}

AddMatrixVolume::AddMatrixVolume( const MatrixField& m1, const MatrixField& m2 ) :
  elem1(m1),
  elem2(m2)
  {}

const Matrix AddMatrixVolume::eval( const Vector& P ) const
{
   return elem1->eval(P) + elem2->eval(P);
}





ExpMatrixVolume::ExpMatrixVolume( Volume<Matrix>* v1 ) : 
   elem1(v1)
   {}

ExpMatrixVolume::ExpMatrixVolume( const MatrixField& v1 ) :
   elem1(v1)
   {}

const Matrix ExpMatrixVolume::eval( const Vector& P ) const
{
   return lux::exp( elem1->eval(P) );
}



