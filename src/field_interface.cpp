#include "field_interface.h"

namespace lux{



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

VolumeSPtr<float> pow(const VolumeSPtr<float>& a, const VolumeSPtr<float>& to_power){
    return std::make_shared<PowField>(a, to_power);
}

vspf union_fields(const vspf a, const vspf b){
    return std::make_shared<UnionField>(std::move(a), std::move(b));
}

vspf intersection(const vspf a, const vspf b){
    return std::make_shared<IntersectionField>(std::move(a), std::move(b));
}

vspf cutout(const vspf a, const vspf b){
    return std::make_shared<CutoutField>(std::move(a), std::move(b));
}

vspf clamp(const vspf a, const vspf min, const vspf max){
    return std::make_shared<ClampField>(std::move(a), std::move(min), std::move(max));
}

vspf mask(const vspf a){
    return std::make_shared<MaskField>(std::move(a));
}

vspf blinn_blend(std::shared_ptr<const std::vector<vspf>> fields, float blend_factor, float shape_broadness){
    return std::make_shared<BlinnBlendField>(std::move(fields), blend_factor, shape_broadness);
}

vspf dilation(vspf a, float d){
    auto eval_func = [a, d](const Vector& p){ return a->eval(p) - d ;};
    auto grad_func = [a](const Vector& p){ return a->grad(p); };
    return funcfield<float>(eval_func, grad_func);
}

vspf shell(vspf a, float d){
    d /= 2.0;
    auto outer = dilation(a, d);
    auto inner = dilation(a, -d);
    return cutout(outer, inner);
}

vspf translate_fixed(const vspf& a, const Vector& delta){
    auto eval_func = [a, &delta](const Vector& p){
        return a->eval(p - delta);
    };
    return funcfield<float>(eval_func);   
}

vspf rotate_fixed(const VolumeSPtr<float> a, const Vector& axis, float angle){
    auto eval_func = [a, &axis, angle](const Vector& p){
        return a->eval(rotation(p, axis, angle));
    };
    return funcfield<float>(eval_func);   
}

// ---------------------------------------------------------------------------------
// fields!
// ---------------------------------------------------------------------------------


VolumeSPtr<float> make_plane(const Vector& point, const Vector& normal) {
    return std::make_shared<PlaneField>(point, normal);
}


VolumeSPtr<float> isf_sphere(const Vector& center, const float radius){
    return std::make_shared<SphereField>(center, radius);
}

VolumeSPtr<float> isf_torus(const Vector& center, float r_major, float r_minor, const Vector& n_hat){
    return std::make_shared<TorusField>(center, r_major, r_minor, n_hat);
}

VolumeSPtr<float> isf_cone(const Vector& x0, const Vector& nhat, float height, float theta){
    return std::make_shared<ConeField>(x0, nhat, height, theta);
}

VolumeSPtr<float> isf_box(const Vector& center, float radius, int rounding_exponent){
    return std::make_shared<BoxField>(center, radius, rounding_exponent);
}


VolumeSPtr<float> isf_icosahedron(const Vector& center){
    return std::make_shared<IcosahedronField>(center);
}

VolumeSPtr<float> isf_steiner_patch(const Vector& center){
    return std::make_shared<SteinerPatchField>(center);
}

VolumeSPtr<float> isf_ellipse(const Vector& center, const Vector& normal, float r_major, float r_minor){
    return std::make_shared<EllipseField>(center, normal, r_major, r_minor);
}


VolumeSPtr<float> isf_cylinder(const Vector& center, const Vector& normal, float r, float h){
    vspf cyl = std::make_shared<CylinderField>(center, normal, r);
    float halfh = h * 0.5;
    auto p0 = make_plane(center + halfh * normal, -normal);
    auto p1 = make_plane(center - halfh * normal, normal);
    cyl = cutout(cyl, p0);
    cyl = cutout(cyl, p1);
    return cyl;
}


} // end namespace lux