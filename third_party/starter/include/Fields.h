
#ifndef __FIELDS_H__
#define __FIELDS_H__

#include "Volume.h"


namespace lux
{


const float  evaluate( const ScalarField& v, const Vector& P );
const Vector evaluate( const VectorField& v, const Vector& P );
const Color  evaluate( const ColorField&  v, const Vector& P );
const Matrix evaluate( const MatrixField&  v, const Vector& P );

//VectorField grad( const ScalarField& e ); 
//MatrixField grad( const VectorField& e ); 

ScalarField constant( const float v );
VectorField constant( const Vector& v );
ColorField  constant( const Color& v );
MatrixField constant( const Matrix& v );

//ScalarField scale( const ScalarField& , const Vector& s );
//VectorField scale( const VectorField& , const Vector& s );
//ColorField  scale( const ColorField& , const Vector& s );
//MatrixField scale( const MatrixField& , const Vector& s );

//ScalarField translate( const ScalarField& , const Vector& s );
//VectorField translate( const VectorField& , const Vector& s );
//ColorField  translate( const ColorField& , const Vector& s );
//MatrixField translate( const MatrixField& , const Vector& s );

//ScalarField rotate( const ScalarField& , const Vector& s );
//VectorField rotate( const VectorField& , const Vector& s );
//ColorField  rotate( const ColorField& , const Vector& s );
//MatrixField rotate( const MatrixField& , const Vector& s );

//MatrixField rotation( const VectorField& v );

ScalarField exp( const ScalarField& v ); 
MatrixField exp( const MatrixField& v ); 

//ScalarField report( const ScalarField& v, const string& tag ); 
//VectorField report( const VectorField& v, const string& tag ); 
//ColorField  report( const ColorField& v, const string& tag ); 
//MatrixField report( const MatrixField& v, const string& tag ); 

//ScalarField negate( const ScalarField& v );
//VectorField negate( const VectorField& v );
//ColorField  negate( const ColorField& v );
//MatrixField negate( const MatrixField& v );

//ScalarField abs( const ScalarField& v ); 
//ScalarField abs( const VectorField& v ); 

//ScalarField which( const ScalarField& v1, const ScalarField& v2, const ScalarField& swtch );
//VectorField which( const VectorField& v1, const VectorField& v2, const ScalarField& swtch );
//ColorField  which( const ColorField& v1, const ColorField& v2, const ScalarField& swtch );
//MatrixField which( const MatrixField& v1, const MatrixField& v2, const ScalarField& swtch );

//ScalarField multiply( const ScalarField& v, const float a ); 
//ScalarField multiply( const ScalarField& v, const ScalarField& u ); 
//VectorField multiply( const VectorField& v, const float a ); 
//VectorField multiply( const VectorField& v, const ScalarField& u ); 
//ColorField  multiply( const ColorField& v, const float a ); 
//ColorField  multiply( const ColorField& v, const ScalarField& u ); 
//ColorField  multiply( const ColorField& v, const ColorField& u ); 
//MatrixField multiply( const MatrixField& v, const float a ); 
//MatrixField multiply( const MatrixField& v, const ScalarField& u ); 

//ScalarField divide( const ScalarField& v, const float a ); 
//ScalarField divide( const ScalarField& v, const ScalarField& u ); 
//VectorField divide( const VectorField& v, const float a ); 
//VectorField divide( const VectorField& v, const ScalarField& u ); 
//ColorField  divide( const ColorField& v, const float a ); 
//ColorField  divide( const ColorField& v, const ScalarField& u ); 
//MatrixField divide( const MatrixField& v, const float a ); 
//MatrixField divide( const MatrixField& v, const ScalarField& u ); 

ScalarField add( const ScalarField&  v1, const ScalarField& v2 );
VectorField add( const VectorField&  v1, const VectorField& v2 );
ColorField  add( const ColorField&  v1, const ColorField& v2 );
MatrixField add( const MatrixField&  v1, const MatrixField& v2 );

//ScalarField subtract( const ScalarField&  v1, const ScalarField& v2 );
//VectorField subtract( const VectorField&  v1, const VectorField& v2 );
//ColorField  subtract( const ColorField&  v1, const ColorField& v2 );
//MatrixField subtract( const MatrixField&  v1, const MatrixField& v2 );


ScalarField Sphere( const Vector& cen, const float rad );
//ScalarField Ellipse( const Vector& cen, const Vector& axs, const float majorrad, const float minorrad ); 
//ScalarField CsgBox( const Vector& cen, const float rad, const float pwr );
//ScalarField Cone( const Vector& cen, const Vector& ax, const float h, const float theta );
//ScalarField Plane( const Vector cen, const Vector norm );
//ScalarField Torus( const Vector& cen, const Vector& axis, const float majorRad, const float minorRad );
//ScalarField SteinerPatch(); 
//ScalarField Icosahedron();
//ScalarField Cylinder( const Vector axis, const float rad );
//ScalarField CappedCylinder( const Vector cen, const Vector axis, const float length, const float radius );
//ScalarField Shell( const ScalarField& v, const float thickness );

//ScalarField mask( const ScalarField& v );
//ScalarField clamp( const ScalarField& v, float minv, float maxv );
//ScalarField pow( const ScalarField& v, float gam );
//ScalarField pow( const ScalarField& v, const ScalarField& gam );
//ColorField  pow( const ColorField& v, float gam );
//ColorField  pow( const ColorField& v, const ScalarField& gam );
//ScalarField Union( const ScalarField& v1, const ScalarField& v2 );
//ScalarField intersection( const ScalarField& v1, const ScalarField& v2 );
//ScalarField cutout( const ScalarField& v1, const ScalarField& v2 );

//ScalarField sin( const ScalarField& v1 );
//ScalarField cos( const ScalarField& v1 );
//ScalarField tan( const ScalarField& v1 );
//ScalarField acos( const ScalarField& v1 );
//ScalarField asin( const ScalarField& v1 );
//ScalarField atan( const ScalarField& v1 );
//ScalarField sinh( const ScalarField& v1 );
//ScalarField cosh( const ScalarField& v1 );
//ScalarField tanh( const ScalarField& v1 );

//MatrixField outer( const VectorField& v1, const VectorField& v2 );
//MatrixField inverse( const MatrixField& m );


//ScalarField gridded( const ScalarGrid& g );
//VectorField gridded( const VectorGrid& g );
//ColorField  gridded( const ColorGrid& g );
//MatrixField gridded( const MatrixGrid& g );


//ScalarField advect( const ScalarField& v, const VectorField& u, const float delt ); 
//VectorField advect( const VectorField& v, const VectorField& u, const float delt ); 
//ColorField  advect( const ColorField& v, const VectorField& u, const float delt ); 
//MatrixField advect( const MatrixField& v, VectorField& u, const float delt ); 

//ScalarField warp( const ScalarField& v, VectorField& map );
//VectorField warp( const VectorField& v, VectorField& map );
//ColorField  warp( const ColorField& v, VectorField& map );
//MatrixField warp( const MatrixField& v, VectorField& map );

//ScalarField dot( const VectorField& e1, const VectorField& e2 );
//VectorField unitvector( const VectorField& e );
//VectorField identity();
//ScalarField xIdentity();
//ScalarField yIdentity();
//ScalarField zIdentity();
//VectorField cross( const VectorField& e1, const VectorField& e2 );
//ScalarField div( const VectorField& e );
//VectorField curl( const VectorField& e );
//VectorField XYZ( const ScalarField& x, const ScalarField& y, const ScalarField& z );

//ColorField Blackbody( const ScalarField& e );
//ColorField RGB( const ScalarField& red, const ScalarField& green, const ScalarField& blue );
//ColorField LUTColor( const ScalarField& field, const vector<Color> lut, const float maxValue, const float minValue );


//ScalarField rComponent( const ColorField& C  );
//ScalarField gComponent( const ColorField& C  );
//ScalarField bComponent( const ColorField& C  );
//ScalarField luminance( const ColorField& C );

//ColorField rgbComponent( const ScalarField& red, const ScalarField& green, const ScalarField& blue );

//ScalarField xComponent( const VectorField& X );
//ScalarField yComponent( const VectorField& X );
//ScalarField zComponent( const VectorField& X );

//VectorField component( const ScalarField& X, const ScalarField& Y, const ScalarField& Z );



}





#endif



