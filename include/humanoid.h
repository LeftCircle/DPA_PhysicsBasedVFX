#pragma once

#include <string>
#include <iostream>
#include <memory>

#include "field_interface.h"
#include "ray_marcher.h"
#include "image_data.h"
#include "command_line_parser.h"


namespace lux{

using isf_col = std::tuple<vspf, vspc>;


void raymarch_humanoid(const std::string& filename);
isf_col human(VolumeSPtr<Color> cf);
isf_col isf_staff(VolumeSPtr<Color> cf, float t = 0);


}