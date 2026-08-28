#pragma once

#include <memory>

#include "volume.h"

namespace lux{

template <typename T>
class FieldOperator : public Volume<T>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	FieldOperator(const VolumeSPtr<T>& a, const VolumeSPtr<T>& b): 
		_a(a), _b(b) {};
	~FieldOperator() = default;

    virtual const volumeDataType eval( const Vector& P ) const override = 0;
   	//virtual const volumeGradType grad( const Vector& P ) const override {}

protected:
	std::shared_ptr<Volume<T>> _a;
	std::shared_ptr<Volume<T>> _b;

};

template <typename T>
class AddFields : public FieldOperator<T>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	AddFields(const VolumeSPtr<T>& a, const VolumeSPtr<T>& b) : FieldOperator<T>(a, b) {}

	const volumeDataType eval (const Vector& p) const override {
		return this->_a->eval(p) + this->_b->eval(p);
	}

	const volumeGradType grad(const Vector& p) const override { return {}; }

};

	
}



