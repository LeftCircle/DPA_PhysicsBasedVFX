

#ifndef __IMPLICITCOLORS_H__
#define __IMPLICITCOLORS_H__


#include "Volume.h"
#include "LinearAlgebra.h"

namespace lux
{

class ConstantColor : public Volume<Color> 
{
  public:

    ConstantColor( const Color& v ) :
       value (v)
    {}

   ~ConstantColor(){}


    const Color eval( const Vector& P ) const 
    {
       return value; 
    }


    virtual std::string typelabel() 
    { 
       std::string lbl = "Constant";
       return lbl;
    }


  private:

    Color value;
};





class AddColor : public Volume<Color> 
{
  public:

    AddColor( Volume<Color> * e, Volume<Color>* v ) :
      elem1(e),
      elem2(v)
    {}

    AddColor( const ColorField& e, const ColorField& v ) :
      elem1(e),
      elem2(v)
    {}

   ~AddColor(){}


    const Color eval( const Vector& P ) const 
    {
       return ( elem1->eval(P) + elem2->eval(P) ); 
    }


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

    ColorField elem1, elem2;
};








}
#endif
