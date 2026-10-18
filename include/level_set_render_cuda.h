#pragma once


#include "level_set_renders.h"
#include "ray_marcher_cuda.h"
#include "render_settings.h"
#include "deep_shadow_map_cuda.h"
#include "lights.h"
#include "cuda_helpers.h"

namespace lux{


void cuda_render_bunny(const std::string& path, const std::string& name);
void cuda_render_tests(const std::string& filename);
void cuda_render_frame(
    const RenderSettings& rs,
    const GridPtr<float> dens_grid,
    const GridPtr<Color> col_grid,
    const std::vector<PointLight>& lights,
    const Camera& cam,
    const std::string& filename
);




} // end namespace lux
