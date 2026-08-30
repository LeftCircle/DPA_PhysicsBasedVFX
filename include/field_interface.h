#pragma once


#include "volume.h"
#include "field_operations.h"
#include "scalar_fields.h"
#include "implicit_fields.h"


namespace lux{


// ---------------------------------------------------------------------------------
// Helper functions
// ---------------------------------------------------------------------------------

template<typename T, typename U>
std::shared_ptr<Volume<T>> add(const VolumeSPtr<T>& a, const VolumeSPtr<U>& b) {
    return std::make_shared<AddFields<T, U>>(a, b);
}


template<typename T, typename U>
std::shared_ptr<Volume<T>> subtract(const VolumeSPtr<T>& a, const VolumeSPtr<U>& b) {
    return std::make_shared<SubtractFields<T, U>>(a, b);
}


VolumeSPtr<float> exp(const VolumeSPtr<float>& a){
    return std::make_shared<ExpField>(a);
}

VolumeSPtr<float> log(const VolumeSPtr<float>& a){
    return std::make_shared<LogField>(a);
}

VolumeSPtr<float> sin(const VolumeSPtr<float>& a){
    return std::make_shared<SinField>(a);
}

VolumeSPtr<float> cos(const VolumeSPtr<float>& a){
    return std::make_shared<CosField>(a);
}

VolumeSPtr<float> pow(const VolumeSPtr<float>& a, const float to_power){
    return std::make_shared<PowField>(a, to_power);
}

template<typename T, typename U>
VolumeSPtr<T> scale(const VolumeSPtr<T>& a, const U& b){
    return std::make_shared<ScaleField<T, U>>(a, b);
}

template<typename T>
VolumeSPtr<T> translate(const VolumeSPtr<T>& a, const Vector& delta){
    return std::make_shared<TranslateField<T>>(a, delta);
}

// ---------------------------------------------------------------------------------
// fields!
// ---------------------------------------------------------------------------------
template<typename T>
VolumeSPtr<T> make_constant(const T& t){
    return std::make_shared<ConstantField<T>>(t);
} 

VolumeSPtr<float> make_plane(const Vector& point, const Vector& normal) {
    return std::make_shared<PlaneField>(point, normal);
}

} // end namespace lux



