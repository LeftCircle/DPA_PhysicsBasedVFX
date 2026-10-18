#pragma once


#include "level_set_renders.h"
#include "ray_marcher_cuda.h"
#include "render_settings.h"
#include "deep_shadow_map_cuda.h"
#include "lights.h"

namespace lux{


void cuda_render_bunny(const std::string& path, const std::string& name);
void cuda_render_noisy_sphere(const std::string& filename);





} // end namespace lux
