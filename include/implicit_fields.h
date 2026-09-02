#pragma once

#include "Vector.h"
#include "volume.h"

namespace lux{


template<typename T>
class ConstantField : public Volume<T> 
{
public:
    using typename Volume<T>::volumeDataType;
    using typename Volume<T>::volumeGradType;

    ConstantField( const T v ) : _value(v) {}
    ~ConstantField() = default;
    
    const volumeDataType eval( const Vector& P ) const override { return _value; }
    const volumeGradType grad( const Vector& P ) const override { return {}; }
    virtual std::string typelabel() { return "Constant"; }
    
private:
    
    volumeDataType _value;
    volumeGradType _gradvalue;
};


class SphereField : public Volume<float>{
public:
    using typename Volume<float>::volumeDataType;
    using typename Volume<float>::volumeGradType;

    SphereField(const Vector& center, const float radius) : _center(center), _radius(radius) {};
    ~SphereField() = default;
    
    const volumeDataType eval( const Vector& p ) const override { return (p - _center).magnitude() - _radius; }
    //const volumeGradType grad( const Vector& p ) const override { return {}; }
    virtual std::string typelabel() { return "Sphere"; }

private:
    Vector _center;
    float _radius;

};

class TorusField : public Volume<float>{
public:
    using volumeDataType = typename Volume<float>::volumeDataType;
    using volumeGradType = typename Volume<float>::volumeGradType;

    TorusField(const Vector& center, float r_major, float r_minor, const Vector& n_hat) : 
            _center(center), _r_major_sq(r_major * r_major), _r_minor_sq(r_minor * r_minor), _n_hat(n_hat) {};
    ~TorusField() = default;
    
    const volumeDataType eval( const Vector& p ) const override;
    //const volumeGradType grad( const Vector& p ) const override { return {}; }
    virtual std::string typelabel() { return "Torus"; }

private:
    Vector _center;
    Vector _n_hat;
    float _r_major_sq;
    float _r_minor_sq;
};


} // end namespace lux