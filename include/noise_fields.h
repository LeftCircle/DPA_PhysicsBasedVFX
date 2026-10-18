#pragma once

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
    auto eval = [r, octaves, translation, freq, fjump, noise_func](const Vector& v) -> float {
            return fractal_sum<float, Vector>(v, noise_func, r, octaves, translation, freq, fjump);
    };
    return std::make_shared<FunctionField<float, decltype(eval)>>(
        std::move(eval)
    ); 
};

}



