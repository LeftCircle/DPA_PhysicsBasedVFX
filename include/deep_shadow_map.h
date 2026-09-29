#pragma once

#include "field_interface.h"
#include "lights.h"
#include "openvdb_helpers.h"

namespace lux{


vspf make_deep_shadow_map(
    vspf density_field,
    float voxel_size,
    coordbbox bounds,
    PointLight light,
    float step_size,
    float kappa
);


template <typename AccumFunc>
float accumulate_over_steps(Vector start, Vector end, float step_size, AccumFunc func, float initial = 0){
    const auto dir = (end - start).unitvector();
    float smax = (end - start).magnitude();
    float s = 0;
    while (s < smax){
        auto pos = start + s * dir;
        initial += func(p);
        s += step_size;
    }
    return initial;
}


} // end namespace lux




