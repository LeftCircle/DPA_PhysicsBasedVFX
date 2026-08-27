#pragma once

#include <memory>

#include "Volume.h"

namespace lux{

template <typename T>
class DoubleHomoField : Volume<t>{
	using vsp = std::shared_ptr<Volume<T>>;
public:
	DoubleHomoField(const vsp& a, const vsp& b): 
		_a(a), _b(b) {};
	~DoubleHomoField() = default;

	const volumeDataType eval( const Vector& P ) const override {
		return _a->eval(P) + _b->eval(P);
	}
   	//const volumeGradType grad( const Vector& P ) const override {}

private:
	std::shared_ptr<Volume<T>> _a;
	std::shared_ptr<Volume<T>> _b;

};



	
}



