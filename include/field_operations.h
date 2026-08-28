#pragma once

#include <memory>

#include "Volume.h"

namespace lux{

template <typename T>
class DoubleHomoField : Volume<T>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	DoubleHomoField(const VolumeSPtr<T>& a, const VolumeSPtr<T>& b): 
		_a(a), _b(b) {};
	~DoubleHomoField() = default;

	//const volumeDataType eval( const Vector& P ) const override = 0;
   	//virtual const volumeGradType grad( const Vector& P ) const override {}

protected:
	std::shared_ptr<Volume<T>> _a;
	std::shared_ptr<Volume<T>> _b;

};

template <typename T>
class AddFields : DoubleHomoField<T>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	AddFields(const VolumeSPtr<T>& a, const VolumeSPtr<T>& b) : DoubleHomoField<T>(a, b) {}

	const volumeDataType eval (const Vector& p) const override {
		return this->_a->eval(p) + this->_b->eval(p);
	}

	const volumeGradType grad(const Vector& p) const override { return {}; }

};

	
}



