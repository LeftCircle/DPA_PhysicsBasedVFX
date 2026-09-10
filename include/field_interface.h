#pragma once

#include <vector>
#include <algorithm>

#include "volume.h"
#include "field_operations.h"
#include "implicit_fields.h"
#include "constructive_solid_geometry.h"

namespace lux{


// ---------------------------------------------------------------------------------
// Helper functions
// ---------------------------------------------------------------------------------

using vspf = VolumeSPtr<float>;

template<typename T, typename U>
std::shared_ptr<Volume<T>> add(const VolumeSPtr<T>& a, const VolumeSPtr<U>& b);

template<typename T, typename U>
std::shared_ptr<Volume<T>> subtract(const VolumeSPtr<T>& a, const VolumeSPtr<U>& b) {
    return std::make_shared<SubtractFields<T, U>>(a, b);
}

template<typename T, typename U>
VolumeSPtr<T> scale(const VolumeSPtr<T>& a, const VolumeSPtr<U>& b){
    return std::make_shared<ScaleField<T, U>>(a, b);
}

vspf scale_fixed(const vspf& a, const Vector& vec_scale);

template<typename T>
VolumeSPtr<T> translate(const VolumeSPtr<T>& a, const VolumeSPtr<Vector>& delta){
    return std::make_shared<TranslateField<T>>(a, delta);
}

vspf translate_fixed(const vspf& a, const Vector& delta);

template<typename T>
VolumeSPtr<T> rotate(const VolumeSPtr<T> a, const VolumeSPtr<Vector> axis, const VolumeSPtr<float> angle){
    return std::make_shared<RotateField<T>>(std::move(a), std::move(axis), std::move(angle));
}

vspf rotate_fixed(const VolumeSPtr<float> a, const Vector& axis, float angle);

float smoothstep(float edge0, float edge1, float x);

Vector smoothstep_lerp(const Vector& a, const Vector& b, float t);


// ---------------------------------------------------------------------------------
// float operations
// ---------------------------------------------------------------------------------
template<typename T>
VolumeSPtr<T> funcfield(
    std::function<T(const Vector&)> eval_func,
    std::function<typename Volume<T>::volumeGradType(const Vector&)> grad_func = {}
){
    return std::make_shared<FunctionField<T>>(std::move(eval_func), std::move(grad_func));
}

VolumeSPtr<float> exp(const VolumeSPtr<float>& a);

VolumeSPtr<float> log(const VolumeSPtr<float>& a);

VolumeSPtr<float> sin(const VolumeSPtr<float>& a);

VolumeSPtr<float> cos(const VolumeSPtr<float>& a);

VolumeSPtr<float> pow(const VolumeSPtr<float>& a, const VolumeSPtr<float>& to_power);

// ---------------------------------------------------------------------------------
// constructive solid geometry
// ---------------------------------------------------------------------------------

vspf union_fields(const vspf a, const vspf b);

vspf intersection(const vspf a, const vspf b);

vspf cutout(const vspf a, const vspf b);

vspf clamp(const vspf a, const vspf min, const vspf max);

vspf mask(const vspf a);

vspf blinn_blend(std::shared_ptr<const std::vector<vspf>> fields, float blend_factor, float shape_broadness);

vspf dilation(const vspf a, float d);

vspf shell(const vspf a, float d);

// ---------------------------------------------------------------------------------
// fields!
// ---------------------------------------------------------------------------------
template<typename T>
VolumeSPtr<T> make_constant(const T& t){
    return std::make_shared<ConstantField<T>>(t);
}

VolumeSPtr<float> make_plane(const Vector& point, const Vector& normal) ;

VolumeSPtr<float> isf_sphere(const Vector& center, const float radius);

VolumeSPtr<float> isf_torus(const Vector& center, float r_major, float r_minor, const Vector& n_hat);

VolumeSPtr<float> isf_cone(const Vector& x0, const Vector& nhat, float height, float theta);

VolumeSPtr<float> isf_box(const Vector& center, float radius, int rounding_exponent);

VolumeSPtr<float> isf_icosahedron(const Vector& center);

VolumeSPtr<float> isf_steiner_patch(const Vector& center);

VolumeSPtr<float> isf_ellipse(const Vector& center, const Vector& normal, float r_major, float r_minor);

VolumeSPtr<float> isf_cylinder(const Vector& center, const Vector& normal, float r, float h);


} // end namespace lux



