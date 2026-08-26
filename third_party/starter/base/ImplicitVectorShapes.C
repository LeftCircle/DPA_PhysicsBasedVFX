
#include "ImplicitVectorShapes.h"

#include <iostream>
using namespace std;
using namespace lux;



AddVectorVolume::AddVectorVolume( const VectorField& v1, const VectorField& v2 ) :
      elem1(v1),
      elem2(v2)
    {}



    const Vector AddVectorVolume::eval( const Vector& P ) const
    {
       return  elem1->eval(P) + elem2->eval(P);
    }

    const Matrix AddVectorVolume::grad( const Vector& P ) const
    {
       return  elem1->grad(P) + elem2->grad(P);
    }





GradientVectorVolume::GradientVectorVolume( Volume<float>* v, const float dx ) :
      elem(v),
      step(dx)
    {}

GradientVectorVolume::GradientVectorVolume( const ScalarField& v, const float dx ) :
      elem(v),
      step(dx)
    {}


const Vector GradientVectorVolume::eval( const Vector& P ) const { return elem->grad(P); }

// evaluate gradient by finite difference
// size of difference is controlled via the "dx" attribute
const Matrix GradientVectorVolume::grad( const Vector& P ) const 
{
   float dx = step;
   Vector exp = eval(P+Vector(dx,0,0));
   Vector eyp = eval(P+Vector(0,dx,0));
   Vector ezp = eval(P+Vector(0,0,dx));
   Vector exm = eval(P-Vector(dx,0,0));
   Vector eym = eval(P-Vector(0,dx,0));
   Vector ezm = eval(P-Vector(0,0,dx));
   Matrix result( exp-exm, eyp-eym, ezp-ezm );
   result /= 2.0*dx;
   return result.transpose();
}



