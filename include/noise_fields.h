#pragma once

#include <cassert>
#include <algorithm>

#include "perlin.h"
#include "volume.h"
#include "fractal_sum_noise.h"
#include "db_perlin.h"
#include "field_operations.h"


namespace lux{


vspf make_fspn(
    float r, // roughness
    float octaves,
    const Vector& translation,
    float freq,
    float fjump
);

vspf make_fspn(const FSVals&& vals);

vspf make_fade_factor(
    const Vector& center,
    float fade,
    float radius
);


// inline vspf make_pyroclastic(
//     const vspf& dens,
//     const vspf&
//     float amplitude, 
//     float gamma
// )

}



