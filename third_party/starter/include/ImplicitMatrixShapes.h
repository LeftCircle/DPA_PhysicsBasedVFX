
#ifndef __IMPLICITMATRIXSHAPES_H__
#define __IMPLICITMATRIXSHAPES_H__

#include "Volume.h"
#include <cmath>
#include <vector>
#include <iostream>



namespace lux
{

class ConstantMatrixVolume: public Volume<Matrix>
{
  public:

    ConstantMatrixVolume( const Matrix& m );
   ~ConstantMatrixVolume(){}

    const Matrix eval( const Vector& P ) const;


    virtual std::string typelabel() 
    { 
       std::string lbl = "Constant";
       return lbl;
    }

  private:

    Matrix elem;

};



class AddMatrixVolume: public Volume<Matrix>
{
  public:

    AddMatrixVolume( Volume<Matrix>* m1, Volume<Matrix>* m2 );
    AddMatrixVolume( const MatrixField& m1, const MatrixField& m2 );
   ~AddMatrixVolume(){}

    const Matrix eval( const Vector& P ) const;


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

    MatrixField elem1, elem2;

};




class ExpMatrixVolume: public Volume<Matrix>
{
  public:

    ExpMatrixVolume( Volume<Matrix>* v1 );
    ExpMatrixVolume( const MatrixField& v1 );
   ~ExpMatrixVolume(){}

    const Matrix eval( const Vector& P ) const;


    virtual std::string typelabel() 
    { 
       std::string lbl = "Exp";
       lbl = lbl + "(";
       lbl = lbl + elem1->typelabel();
       lbl = lbl + ")";
       return lbl;
    }

  private:

    MatrixField elem1;

};


}
#endif
