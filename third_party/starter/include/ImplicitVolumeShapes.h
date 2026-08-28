
// #ifndef __IMPLICITVOLUMESHAPES_H__
// #define __IMPLICITVOLUMESHAPES_H__

// #include "Volume.h"
// #include "LinearAlgebra.h"
// #include <cmath>
// #include <vector>
// #include <iostream>



// namespace lux
// {


// class ConstantVolume : public Volume<float> 
// {
//   public:

//     ConstantVolume( const float v ) ;

//    ~ConstantVolume(){}


//     const float eval( const Vector& P ) const; 

//     const Vector grad( const Vector& P ) const; 

//    virtual std::string typelabel() { return "Constant"; }


//   private:

//     float value;
//     Vector gradvalue;
// };

// template<typename T>
// class ConstantField : public Volume<T> 
// {
// public:
//     using typename Volume<T>::volumeDataType;
//     using typename Volume<T>::volumeGradType;

//     ConstantField( const T v ) : _value(v), _gradvalue{} {}
//     ~ConstantField() = default;
    
//     const volumeDataType eval( const Vector& P ) const override { return _value; }
//     const volumeGradType grad( const Vector& P ) const override { return {}; }
//     virtual std::string typelabel() { return "Constant"; }
    
// private:
    
//     volumeDataType _value;
//     volumeGradType _gradvalue;
// };


// class ExpVolume: public Volume<float> 
// {
//   public:

//     ExpVolume( Volume<float>* v );

//     ExpVolume( const ScalarField& v );

//    ~ExpVolume(){}


//     const float eval( const Vector& P ) const; 

//     const Vector grad(  const Vector& P ) const;

//    virtual std::string typelabel() 
//    { 
//       std::string lbl = "Exp";
//       lbl = lbl + "(";
//       lbl = lbl + elem->typelabel();
//       lbl = lbl + ")";
//       return lbl;
//    }

//   private:

//     const ScalarField elem;

// };




// class AddVolume : public Volume<float> 
// {
//   public:

//     AddVolume( Volume<float> * v1, Volume<float> * v2 );

//     AddVolume( const ScalarField&  v1, const ScalarField& v2 );

//     ~AddVolume(){}


//     const float eval( const Vector& P ) const;

//     const Vector grad(  const Vector& P ) const;

//    virtual std::string typelabel() 
//    { 
//       std::string lbl = "Add";
//       lbl = lbl + "(";
//       lbl = lbl + e1->typelabel();
//       lbl = lbl + ",";
//       lbl = lbl + e2->typelabel();
//       lbl = lbl + ")";
//       return lbl;
//    }

//   private:

//     const ScalarField e1, e2;
// };





// class SphereVolume : public Volume<float> 
// {
//   public:

//     SphereVolume( const Vector& cen, const float rad );

//    ~SphereVolume(){}


//     const float eval( const Vector& P ) const; 

//     const Vector grad(  const Vector& P ) const; 

//     virtual std::string typelabel() { return std::string("Sphere"); }


//   private:

//     Vector center;
//     float radius;
// };




// template <typename T>
// class DoubleHomoField : Volume<T>{
// public:
// 	using typename Volume<T>::volumeDataType;
// 	using typename Volume<T>::volumeGradType;

// 	DoubleHomoField(const VolumeSPtr<T>& a, const VolumeSPtr<T>& b): 
// 		_a(a), _b(b) {};
// 	~DoubleHomoField() = default;

// 	//const volumeDataType eval( const Vector& P ) const override = 0;
//    	//virtual const volumeGradType grad( const Vector& P ) const override {}

// protected:
// 	std::shared_ptr<Volume<T>> _a;
// 	std::shared_ptr<Volume<T>> _b;

// };

// template <typename T>
// class AddFields : DoubleHomoField<T>{
// public:
// 	using typename Volume<T>::volumeDataType;
// 	using typename Volume<T>::volumeGradType;

// 	AddFields(const VolumeSPtr<T>& a, const VolumeSPtr<T>& b) : DoubleHomoField<T>(a, b) {}

// 	const volumeDataType eval (const Vector& p) const override {
// 		return this->_a->eval(p) + this->_b->eval(p);
// 	}

// 	const volumeGradType grad(const Vector& p) const override { return {}; }

// };



// }





// #endif



