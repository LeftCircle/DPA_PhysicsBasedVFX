#pragma once

#include <memory>
#include <type_traits>

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

// NOTE -> the first type is used to determine the gradient type. 
// ie <vector, float> will return a gradient of a matrix
template <typename T, typename U>
class MultiplyFields : public FieldOperator<T, U>{
	static_assert(
		!(std::is_same_v<T, float> && std::is_same_v<U, Vector>),
		"Multiplication only supports <Vector, float>, not <float, Vector>"
	);
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	MultiplyFields(const VolumeSPtr<T>& a, const VolumeSPtr<U>& b) : FieldOperator<T, U>(a, b) {}

	const volumeDataType eval (const Vector& p) const override {
		return this->_a->eval(p) * this->_b->eval(p);
	}

	const volumeGradType grad(const Vector& p) const override { 
        return this->_a->grad(p) * this->_b->eval(p) + this->_a->eval(p) * this->_b->grad(p);
    }
};

template <typename T, typename U>
class DivideFields : public FieldOperator<T, U>{
	static_assert(
		!(std::is_same_v<T, float> && std::is_same_v<U, Vector>),
		"Divide only supports <Vector, float>, not <float, Vector>"
	);
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	DivideFields(const VolumeSPtr<T>& a, const VolumeSPtr<U>& b) : FieldOperator<T, U>(a, b) {}

	const volumeDataType eval (const Vector& p) const override {
		return this->_a->eval(p) / this->_b->eval(p);
	}

	const volumeGradType grad(const Vector& p) const override {
		auto b_eval = this->_b->eval(p);
		if (b_eval == U{}){
			return {};
		} else{
			return this->_a->grad(p) / b_eval + this->_b->grad(p) * this->_a->eval(p) / (b_eval * b_eval);
		}
    }
};

// ---------------------------------------------------------------------------------
// Operators requiring a single field
// ---------------------------------------------------------------------------------

template <typename T>
class SingleFieldOperator : public Volume<T>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	SingleFieldOperator(const VolumeSPtr<T>& a): _a(a) {};
	~SingleFieldOperator() = default;

    virtual const volumeDataType eval( const Vector& P ) const override = 0;
   	//virtual const volumeGradType grad( const Vector& P ) const override {}

protected:
	std::shared_ptr<Volume<T>> _a;

};


template <typename T>
class NegateField : public SingleFieldOperator<T>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	NegateField(const VolumeSPtr<T>& a) : SingleFieldOperator<T>(a) {}

	const volumeDataType eval (const Vector& p) const override {
		return -this->_a->eval(p);
	}

	const volumeGradType grad(const Vector& p) const override { 
        return -this->_a->grad(p);
    }
};

class ExpField : public SingleFieldOperator<float>{
public:
	using typename Volume<float>::volumeDataType;
	using typename Volume<float>::volumeGradType;

	ExpField(const VolumeSPtr<float>& a) : SingleFieldOperator<float>(a) {}

	const volumeDataType eval (const Vector& p) const override {
		return std::exp(this->_a->eval(p));
	}

	const volumeGradType grad(const Vector& p) const override { 
        return std::exp(this->_a->eval(p)) * this->_a->grad(p);
    }
};

class LogField : public SingleFieldOperator<float>{
public:
	using typename Volume<float>::volumeDataType;
	using typename Volume<float>::volumeGradType;

	LogField(const VolumeSPtr<float>& a) : SingleFieldOperator<float>(a) {}

	const volumeDataType eval (const Vector& p) const override {
		return std::log(this->_a->eval(p));
	}

	const volumeGradType grad(const Vector& p) const override { 
        auto a_eval = this->_a->eval(p);
		if (a_eval == 0.0){
			return {};
		} else{
			return this->_a->grad(p) / a_eval;
		}
    }
};

class SinField : public SingleFieldOperator<float>{
public:
	using typename Volume<float>::volumeDataType;
	using typename Volume<float>::volumeGradType;

	SinField(const VolumeSPtr<float>& a) : SingleFieldOperator<float>(a) {}

	const volumeDataType eval (const Vector& p) const override {
		return std::sin(this->_a->eval(p));
	}

	const volumeGradType grad(const Vector& p) const override { 
        return std::cos(this->_a->eval(p)) * this->_a->grad(p);
    }
};

class CosField : public SingleFieldOperator<float>{
public:
	using typename Volume<float>::volumeDataType;
	using typename Volume<float>::volumeGradType;

	CosField(const VolumeSPtr<float>& a) : SingleFieldOperator<float>(a) {}

	const volumeDataType eval (const Vector& p) const override {
		return std::cos(this->_a->eval(p));
	}

	const volumeGradType grad(const Vector& p) const override { 
        return -std::sin(this->_a->eval(p)) * this->_a->grad(p);
    }
};


// ---------------------------------------------------------------------------------
// Operators requiring a single field a non field value
// ---------------------------------------------------------------------------------

template <typename T, typename U>
class FieldValOperator : public Volume<T>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	FieldValOperator(const VolumeSPtr<T>& a, const U& b): _a(a), _b(b) {};
	~FieldValOperator() = default;

    virtual const volumeDataType eval( const Vector& P ) const override = 0;
   	//virtual const volumeGradType grad( const Vector& P ) const override {}

protected:
	std::shared_ptr<Volume<T>> _a;
	U _b;

};

class PowField : public FieldValOperator<float, float>{
public:
	using typename Volume<float>::volumeDataType;
	using typename Volume<float>::volumeGradType;

	PowField(const VolumeSPtr<float>& a, const float to_power) : FieldValOperator<float, float>(a, to_power) {}

	const volumeDataType eval (const Vector& p) const override {
		return std::pow(this->_a->eval(p), this->_b);
	}

	const volumeGradType grad(const Vector& p) const override { 
        return this->_b * std::pow(this->_a->eval(p), this->_b - 1.0) * this->_a->grad(p);
    }
};

template<typename T, typename U>
class ScaleField : public FieldValOperator<T, U>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	ScaleField(const VolumeSPtr<T>& a, const U& scale) : FieldValOperator<T, U>(a, scale) {}

	const volumeDataType eval (const Vector& p) const override {
		return this->_a->eval(p / this->_b);
	}

	const volumeGradType grad(const Vector& p) const override { 
        if (this->_b == U{}){
			return {};
		} else{
			return this->_a->grad(p / this->_b) / this->_b;
		}
    }
};

template<typename T>
class TranslateField : public FieldValOperator<T, Vector>{
public:
	using typename Volume<T>::volumeDataType;
	using typename Volume<T>::volumeGradType;

	TranslateField(const VolumeSPtr<T>& a, const Vector& delta_x) : FieldValOperator<T, Vector>(a, delta_x) {}

	const volumeDataType eval (const Vector& p) const override {
		return this->_a->eval(p - this->_b);
	}

	const volumeGradType grad(const Vector& p) const override { 
        return this->_a->grad(p - this->_b);
    }
};


// ---------------------------------------------------------------------------------
// Operator overloading shared pointer stuff double fields
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

template <typename T, typename U>
std::shared_ptr<Volume<T>> operator/(const VolumeSPtr<T>& a,
                                     const VolumeSPtr<U>& b)
{
    return std::make_shared<DivideFields<T, U>>(a, b);
}

inline std::shared_ptr<Volume<Vector>> operator/(const VolumeSPtr<float>& a,
                                     const VolumeSPtr<Vector>& b)
{
    return std::make_shared<DivideFields<Vector, float>>(b, a);
}

inline std::shared_ptr<Volume<Vector>> operator/(const VolumeSPtr<Vector>& a,
                                     const VolumeSPtr<float>& b)
{
    return std::make_shared<DivideFields<Vector, float>>(a, b);
}


// ---------------------------------------------------------------------------------
// Operator overloading shared pointer stuff single fields
// ---------------------------------------------------------------------------------

template <typename T>
std::shared_ptr<Volume<T>> operator-(const VolumeSPtr<T>& a)
{
    return std::make_shared<NegateField<T>>(a);
}


} // end namespace lux



