
#ifndef __IMPLICITVECTORSHAPES_H__
#define __IMPLICITVECTORSHAPES_H__

#include "Volume.h"
#include "LinearAlgebra.h"
#include <cmath>
#include <vector>
#include <iostream>

namespace lux
{



class ConstantVectorVolume : public Volume<Vector> 
{
  public:

    ConstantVectorVolume( const Vector v ) ;

   ~ConstantVectorVolume(){}


    const Vector eval( const Vector& P ) const; 

    const Matrix grad( const Vector& P ) const;

    virtual std::string typelabel() 
    { 
       std::string lbl = "Constant";
       return lbl;
    }


  private:

    Vector value;
    Matrix gradvalue;
};

class AddVectorVolume : public Volume<Vector> 
{
  public:

    AddVectorVolume( Volume<Vector> * v1, Volume<Vector> * v2 ) ;

    AddVectorVolume( const VectorField& v1, const VectorField& v2 ) ;

    ~AddVectorVolume(){}


    const Vector eval( const Vector& P ) const;

    const Matrix grad( const Vector& P ) const;


    virtual std::string typelabel() 
    { 
       std::string lbl = "Add";
       lbl = lbl + "(";
       lbl = lbl + elem1->typelabel();
       lbl = lbl + ",";
       lbl = lbl + elem2->typelabel();
       lbl = lbl + ")";
       return lbl;
    }

  private:
  
    VectorField elem1, elem2;
};


class GradientVectorVolume : public Volume<Vector> 
{
  public:

    GradientVectorVolume( Volume<float>* v, const float dx = 0.001 ) ;

    GradientVectorVolume( const ScalarField& v, const float dx = 0.001 ) ;

    ~GradientVectorVolume(){}

    const Vector eval( const Vector& P ) const;

    // evaluate gradient by finite difference
    // size of difference is controlled via the "dx" attribute
    const Matrix grad( const Vector& P ) const ;


    virtual std::string typelabel() 
    { 
       std::string lbl = "Grad";
       lbl = lbl + "(";
       lbl = lbl + elem->typelabel();
       lbl = lbl + ")";
       return lbl;
    }

  private:

    ScalarField elem;
    float step;
};




}
#endif
