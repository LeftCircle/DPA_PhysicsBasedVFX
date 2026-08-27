#pragma once

#include "Vector.h"
#include "Volume.h"

namespace lux{


template<typename T>
class ConstantField : public Volume<T> 
{
public:
    ConstantField( const T v ) : _value(v), _gradvalue(v * 0) {}
    ~ConstantField() = default;
    
    const volumeDataType eval( const Vector& P ) const override { return _value; } 
    const volumeGradType grad( const Vector& P ) const override { return 0; } 
    virtual std::string typelabel() { return "Constant"; }
    
private:
    
    volumeDataType _value;
    volumeGradType _gradvalue;
};


}