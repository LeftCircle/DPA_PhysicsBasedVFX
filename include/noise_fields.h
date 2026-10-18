#pragma once

#include <cassert>
#include <algorithm>

#include "volume.h"
#include "fractal_sum_noise.h"
#include "db_perlin.h"
#include "field_operations.h"


namespace lux{

inline vspf make_fspn(
    float r, // roughness
    float octaves,
    const Vector& translation,
    float freq,
    float fjump
){
    auto noise_func = [](const Vector& v){
        return db::perlin<float>(v.x(), v.y(), v.z());
    };
    auto eval = [r, octaves, translation, freq, fjump, noise_func](const Vector& v){
            return fractal_sum<float, Vector>(v, noise_func, r, octaves, translation, freq, fjump);
    };
    return std::make_shared<FunctionField<float, decltype(eval)>>(
        std::move(eval)
    ); 
};

inline vspf make_fade_factor(
    const Vector& center,
    float fade,
    float radius
){
    assert(fade >= 0);
    float rsq = radius * radius;
    auto eval = [center, fade, rsq](const Vector& v){
        float q = (1.0 - (center - v).magnitude_squared() / rsq) / fade;
        return std::clamp(q, 0.0f, 1.0f);
    };
    return std::make_shared<FunctionField<float, decltype(eval)>>(std::move(eval));
}

// inline vspf make_pyroclastic(
//     const vspf& dens,
//     const vspf&
//     float amplitude, 
//     float gamma
// )

}



