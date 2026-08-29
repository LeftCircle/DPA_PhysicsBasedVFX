#pragma once

#include <memory>

#include "volume.h"

namespace lux{

// ---------------------------------------------------------------------------------
// Templated field operators
// --------------------------------------------------------------------------------- 

template <typename T, typename U>
class FieldOperator : public Volume<T>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	FieldOperator(const VolumeSPtr<T>& a, const VolumeSPtr<U>& b): 
		_a(a), _b(b) {};
	~FieldOperator() = default;

    virtual const volumeDataType eval( const Vector& P ) const override = 0;

   	// Keeping base class grad implementation and allowing child classes to override
    //virtual const volumeGradType grad( const Vector& P ) const override {}

protected:
	std::shared_ptr<Volume<T>> _a;
	std::shared_ptr<Volume<U>> _b;

};

template <typename T>
class SingleFieldOperator : public Volume<T>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	SingleFieldOperator(const VolumeSPtr<T>& a, const VolumeSPtr<T>& b): _a(a) {};
	~SingleFieldOperator() = default;

    virtual const volumeDataType eval( const Vector& P ) const override = 0;
   	//virtual const volumeGradType grad( const Vector& P ) const override {}

protected:
	std::shared_ptr<Volume<T>> _a;

};

// Pull grad type from T
template <typename T, typename U>
class AddFields : public FieldOperator<T, U>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	AddFields(const VolumeSPtr<T>& a, const VolumeSPtr<T>& b) : FieldOperator<T, U>(a, b) {}

	const volumeDataType eval (const Vector& p) const override {
		return this->_a->eval(p) + this->_b->eval(p);
	}

	const volumeGradType grad(const Vector& p) const override { return this->_a->grad(p) + this->_b->grad(p); }

};

template <typename T, typename U>
class SubtractFields : public FieldOperator<T, U>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	SubtractFields(const VolumeSPtr<T>& a, const VolumeSPtr<U>& b) : FieldOperator<T, U>(a, b) {}

	const volumeDataType eval (const Vector& p) const override {
		return this->_a->eval(p) - this->_b->eval(p);
	}

	const volumeGradType grad(const Vector& p) const override { return this->_a->grad(p) - this->_b->grad(p); }

};

template <typename T, typename U>
class MultiplyFields : public FieldOperator<T, U>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	MultiplyFields(const VolumeSPtr<T>& a, const VolumeSPtr<U>& b) : FieldOperator<T, U>(a, b) {}

	const volumeDataType eval (const Vector& p) const override {
		return this->_a->eval(p) * this->_b->eval(p);
	}

	const volumeGradType grad(const Vector& p) const override { 
        return {this->_a->grad(p) * this->_b->eval(p) + this->_a->eval(p) * this->_b->grad(p)};
    }

};

// ---------------------------------------------------------------------------------
// Operator overloading shared pointer stuff
// ---------------------------------------------------------------------------------

template <typename T, typename U>
std::shared_ptr<Volume<T>> operator+(
    const VolumeSPtr<T>& a,
    const VolumeSPtr<U>& b
){
    return std::make_shared<AddFields<T, U>>(a, b);
}

template <typename T, typename U>
std::shared_ptr<Volume<T>> operator-(const VolumeSPtr<T>& a,
                                     const VolumeSPtr<U>& b)
{
    return std::make_shared<SubtractFields<T, U>>(a, b);
}


template <typename T, typename U>
std::shared_ptr<Volume<T>> operator*(const VolumeSPtr<T>& a,
                                     const VolumeSPtr<U>& b)
{
    return std::make_shared<MultiplyFields<T, U>>(a, b);
}

inline std::shared_ptr<Volume<Vector>> operator*(const VolumeSPtr<float>& a,
                                     const VolumeSPtr<Vector>& b)
{
    return std::make_shared<MultiplyFields<Vector, float>>(b, a);
}

inline std::shared_ptr<Volume<Vector>> operator*(const VolumeSPtr<Vector>& a,
                                     const VolumeSPtr<float>& b)
{
    return std::make_shared<MultiplyFields<Vector, float>>(a, b);
}

} // end namespace lux



