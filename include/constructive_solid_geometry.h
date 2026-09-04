#pragma once


#include "volume.h"
#include "field_operations.h"


namespace lux{


class UnionField : public FieldOperator<float, float>{
public:
	using typename Volume<float>::volumeDataType;
	using typename Volume<float>::volumeGradType;

	UnionField(const VolumeSPtr<float> a, const VolumeSPtr<float> b) : 
						FieldOperator<float, float>(std::move(a), std::move(b)) {}

	const volumeDataType eval(const Vector& p) const override {
		return std::min(this->_a->eval(p), std::get<0>(this->_values)->eval(p));
	}

	const volumeGradType grad(const Vector& p) const override { 
        if (this->_a->eval(p) > std::get<0>(this->_values)->eval(p)){
            return this->_a->grad(p);
        } else{
            return std::get<0>(this->_values)->grad(p);
        }
    }
};


class IntersectionField : public FieldOperator<float, float>{
public:
	using typename Volume<float>::volumeDataType;
	using typename Volume<float>::volumeGradType;

	IntersectionField(const VolumeSPtr<float> a, const VolumeSPtr<float> b) : 
						FieldOperator<float, float>(std::move(a), std::move(b)) {}

	const volumeDataType eval(const Vector& p) const override {
		return std::max(this->_a->eval(p), std::get<0>(this->_values)->eval(p));
	}

	const volumeGradType grad(const Vector& p) const override { 
        if (this->_a->eval(p) < std::get<0>(this->_values)->eval(p)){
            return this->_a->grad(p);
        } else{
            return std::get<0>(this->_values)->grad(p);
        }
    }
};


class CutoutField : public FieldOperator<float, float>{
public:
	using typename Volume<float>::volumeDataType;
	using typename Volume<float>::volumeGradType;

	CutoutField(const VolumeSPtr<float> a, const VolumeSPtr<float> b) : 
						FieldOperator<float, float>(std::move(a), std::move(b)) {}

	const volumeDataType eval(const Vector& p) const override {
		return std::max(this->_a->eval(p), -std::get<0>(this->_values)->eval(p));
	}

	const volumeGradType grad(const Vector& p) const override { 
        if (this->_a->eval(p) > -std::get<0>(this->_values)->eval(p)){
            return this->_a->grad(p);
        } else{
            return std::get<0>(this->_values)->grad(p);
        }
    }
};

class MaskField : public FieldOperator<float>{
public:
	using typename Volume<float>::volumeDataType;
	using typename Volume<float>::volumeGradType;

	MaskField(const VolumeSPtr<float> a) : 
						FieldOperator<float>(std::move(a)) {}

	const volumeDataType eval(const Vector& p) const override {
		return this->_a->eval(p) < 0 ? 1.0 : 0.0;
	}
};


class ClampField : public FieldOperator<float, float, float>{
public:
	using typename Volume<float>::volumeDataType;
	using typename Volume<float>::volumeGradType;

	ClampField(const VolumeSPtr<float> a, const VolumeSPtr<float> min, const VolumeSPtr<float> max) : 
						FieldOperator<float, float, float>(std::move(a), std::move(min), std::move(max)) {}

	const volumeDataType eval(const Vector& p) const override {
		auto eval = this->_a->eval(p);
		auto min = std::get<0>(this->_values)->eval(p);
		auto max = std::get<0>(this->_values)->eval(p);
		if (eval < min){
			return min;
		} else if (eval > max){
			return max;
		} else{
			return eval;
		}
	}
};





} // end namespace lux




