#pragma once

#include <string>
#include <iostream>
#include <memory>

#include "field_interface.h"
#include "ray_marcher.h"
#include "image_data.h"
#include "command_line_parser.h"


namespace lux{


void raymarch_humanoid(const std::string& filename);


}