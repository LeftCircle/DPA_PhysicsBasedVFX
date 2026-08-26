
#ifndef __VOLUME_H__
#define __VOLUME_H__

#include "Vector.h"
#include "Matrix.h"
#include "Color.h"
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <cstring>

#include <math.h>

#include <iostream>

namespace lux
{

class VolumeBase
{
  public:

    VolumeBase(){}
    virtual ~VolumeBase(){}


    void addAttribute( const std::string& name, const float value )
    {
       std::map<std::string, float>::const_iterator check = volumeAttributes.find( name );
       if( check == volumeAttributes.end() )
       {
          volumeAttributes[ name ] = value;
       }
    }

    const float attribute( const std::string& name ) const
    {
       std::map<std::string,float>::const_iterator check = volumeAttributes.find( name );
       if( check != volumeAttributes.end() )
       {
          return (*check).second;
       }
       return 0;
    }

  protected:

    std::map<std::string, float> volumeAttributes;

};

//-----------------------------------------------------------------------------



class FDGradHandler
{
  public:

    FDGradHandler();
 
    ~FDGradHandler(){}

    void setNbTerms( const int n )
    {
       if( n <= (int)_grad_coefficients.size() && n > 0 )
       {
          _nb_grad = n;
       }
    }

    const int nbTerms() const { return _nb_grad; }

    void setStep( const double dx )
    {
       if( dx > 0 ){ _dx = dx; _dy = dx; _dz = dx; }
    }

    void setStep( const double dx, const double dy, const double dz )
    {
       if( dx > 0 && dy > 0 && dz > 0 ){ _dx = dx; _dy = dy; _dz = dz; }
    }

    const double step() const { return _dx; }
    const double step_x() const { return _dx; }
    const double step_y() const { return _dy; }
    const double step_z() const { return _dz; }

    const double coefficient( const int n ) const
    {
       if( n > 0 && n <= _nb_grad ){ return _grad_coefficients[_nb_grad-1][n-1]; }
       return 0.0;
    }


  private:
    int _nb_grad;
    double _dx,_dy,_dz;
    std::vector< std::vector<double> > _grad_coefficients;
};







//-----------------------------------------------------------------------------
// Setting up logic to be able to determine the data type of the gradient 
template <typename U>
struct GradType
{
   typedef int GType;
};

template<>
struct GradType<float>
{
   typedef Vector GType;
};

template<>
struct GradType<Vector>
{
   typedef Matrix GType;
};



//-----------------------------------------------------------------------------

template<typename U, typename G>
const G FDGradient( const U& x, const U& y, const U& z )
{
   return 0;
};

template<>
const Vector FDGradient( const float& x, const float& y, const float& z );

template<>
const Matrix FDGradient( const Vector& x, const Vector& y, const Vector& z );



//-----------------------------------------------------------------------------

template< typename U >
class Volume : public VolumeBase
{
  public:

    Volume(){}
    virtual ~Volume(){}


   typedef U volumeDataType;
   typedef typename GradType<U>::GType volumeGradType;

   virtual const volumeDataType eval( const Vector& P ) const { volumeDataType base; base = base - base; return base; }
   virtual const volumeGradType grad( const Vector& P ) const 
   {
      volumeDataType valueX, valueY, valueZ;
      valueX = valueX - valueX;
      valueY = valueY - valueY;
      valueZ = valueZ - valueZ;
      Vector dx(gradParams.step_x(),0,0), dy(0,gradParams.step_y(),0), dz(0,0,gradParams.step_z());
      for( size_t i=1;i<=(size_t)gradParams.nbTerms();i++ )
      {
         double coeff = gradParams.coefficient( (int)i );
	 if(isnan(coeff)){ std::cout << "Volume grad NAN " << i << "   dx " << dx.X() << " " << dx.Y() << " " << dx.Z() << std::endl; }
         valueX += (eval( P + i*dx ) - eval( P - i*dx ) ) * coeff / gradParams.step_x();
         valueY += (eval( P + i*dy ) - eval( P - i*dy ) ) * coeff / gradParams.step_y();
         valueZ += (eval( P + i*dz ) - eval( P - i*dz ) ) * coeff / gradParams.step_z();
      }
      return FDGradient<volumeDataType, volumeGradType>(valueX, valueY, valueZ); 
   }

   virtual std::string typelabel() { return "unknown"; }
   virtual std::string documentation() { return "No documentation"; }

   FDGradHandler gradParams;

   void setFDSize( int nb ){ gradParams.setNbTerms( nb ); }
   void setFDStep( double dx ){ gradParams.setStep( dx ); }
   void setFDStep( double dx, double dy, double dz ){ gradParams.setStep( dx, dy, dz); }
};

typedef Volume<float>* VolumeFloatPtr;
typedef Volume<Color>* VolumeColorPtr;
typedef Volume<Vector>* VolumeVectorPtr;
typedef Volume<Matrix>* VolumeMatrixPtr;

typedef std::shared_ptr<Volume<float> > ScalarFieldBase;
typedef std::shared_ptr<Volume<Vector> > VectorFieldBase;
typedef std::shared_ptr<Volume<Color> > ColorFieldBase;
typedef std::shared_ptr<Volume<Matrix> > MatrixFieldBase;

class VectorField;
class ColorField;
class MatrixField;

class ScalarField : public ScalarFieldBase
{
  public:

    ScalarField();
    ScalarField( Volume<float>* f );
   ~ScalarField();

     char* __str__() 
     {
       static char typeLabel[2048];
       std::string lbl = (*this)->typelabel();
       size_t lbllength = lbl.size();
       if( lbllength > 2047 ){ lbllength = 2047; }
       lbllength = lbl.copy( typeLabel, lbllength);
       typeLabel[lbllength] = '\0';
       return typeLabel;
    }


     char* __doc__() 
     {
       static char docLabel[2048];
       std::string lbl = (*this)->documentation();
       size_t lbllength = lbl.size();
       if( lbllength > 2047 ){ lbllength = 2047; }
       lbllength = lbl.copy( docLabel, lbllength);
       docLabel[lbllength] = '\0';
       return docLabel;
    }



    ScalarField operator+( const ScalarField& e2 );
    ScalarField operator-( const ScalarField& e2 );
    ScalarField operator-();
    ScalarField operator*( const ScalarField& e2 );
    ScalarField operator/( const ScalarField& e2 );
    ScalarField operator^(  const ScalarField& e2 );
    ScalarField operator&&( const ScalarField& e2 );
    ScalarField operator||( const ScalarField& e2 );
    VectorField operator*( const VectorField& e1 );
    ColorField operator*( const ColorField& e1 );

    MatrixField operator*( const MatrixField& e1 );

};

class VectorField : public VectorFieldBase
{
  public:

    VectorField();
    VectorField( Volume<Vector>* f );
   ~VectorField();

    char* __str__() 
    {
       static char typeLabel[2048];
       std::string lbl = (*this)->typelabel();
       size_t lbllength = lbl.size();
       if( lbllength > 2047 ){ lbllength = 2047; }
       lbllength = lbl.copy( typeLabel, lbllength);
       typeLabel[lbllength] = '\0';
       return typeLabel;
    }

     char* __doc__() 
     {
       static char docLabel[2048];
       std::string lbl = (*this)->documentation();
       size_t lbllength = lbl.size();
       if( lbllength > 2047 ){ lbllength = 2047; }
       lbllength = lbl.copy( docLabel, lbllength);
       docLabel[lbllength] = '\0';
       return docLabel;
    }



    VectorField operator+( const VectorField& e2 );
    VectorField operator-( const VectorField& e2 );
    VectorField operator-( );
    VectorField operator*( const ScalarField& e2 );
    ScalarField operator*( const VectorField& e2 );
    VectorField operator/( const ScalarField& e2 );
    VectorField operator^( const VectorField& e2 );

    VectorField operator*( const MatrixField& e2 );


};

class ColorField : public ColorFieldBase
{
  public:

    ColorField();
    ColorField( Volume<Color>* f );
   ~ColorField();

    char* __str__() 
    {
       static char typeLabel[2048];
       std::string lbl = (*this)->typelabel();
       size_t lbllength = lbl.size();
       if( lbllength > 2047 ){ lbllength = 2047; }
       lbllength = lbl.copy( typeLabel, lbllength);
       typeLabel[lbllength] = '\0';
       return typeLabel;
    }


    ColorField operator+( const ColorField& e2 );
    ColorField operator-( const ColorField& e2 );
    ColorField operator-( );
    ColorField operator*( const ColorField& e2 );
    ColorField operator*( const ScalarField& e2 );
    ColorField operator/( const ScalarField& e2 );

};




class MatrixField : public MatrixFieldBase
{
  public:

    MatrixField();
    MatrixField( Volume<Matrix>* f );
   ~MatrixField();

    char* __str__() 
    {
       static char typeLabel[2048];
       std::string lbl = (*this)->typelabel();
       size_t lbllength = lbl.size();
       if( lbllength > 2047 ){ lbllength = 2047; }
       lbllength = lbl.copy( typeLabel, lbllength);
       typeLabel[lbllength] = '\0';
       return typeLabel;
    }

     char* __doc__() 
     {
       static char docLabel[2048];
       std::string lbl = (*this)->documentation();
       size_t lbllength = lbl.size();
       if( lbllength > 2047 ){ lbllength = 2047; }
       lbllength = lbl.copy( docLabel, lbllength);
       docLabel[lbllength] = '\0';
       return docLabel;
    }




    MatrixField operator+( const MatrixField& e2 );
    MatrixField operator-( const MatrixField& e2 );
    MatrixField operator-();
    MatrixField operator*( const ScalarField& e2 );
    VectorField operator*( const VectorField& e2 );
    MatrixField operator*( const MatrixField& e2 );
    MatrixField operator/( const ScalarField& e2 );

};



ScalarField SF( Volume<float>* v );  
VectorField VF( Volume<Vector>* v ); 
ColorField  CF( Volume<Color>* v );  
MatrixField MF( Volume<Matrix>* v ); 



}

#endif
