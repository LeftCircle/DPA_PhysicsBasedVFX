#pragma once


#include <string>
#include <iostream>
#include <memory>

#include "field_interface.h"
#include "ray_marcher.h"
#include "image_data.h"
#include "command_line_parser.h"


#include "openvdb_helpers.h"


#include "string_funcs.h"

#include <openvdb/openvdb.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>

#include "obj_reader.h"

namespace lux{

#define RADTODEG(radians) ((radians) * 180.0f / 3.14159265358979)
#define DEGTORAD(degrees) ((degrees * 3.14159265358979 / 180.0))

using vecsptr = std::shared_ptr<const std::vector<vspf>>;


void render_bunny(const std::string& path, const std::string& name);

void render_turnable(
    const std::string& path,
    const std::string& name, 
    const vspf& density,
    const vspf& masked_density,
    const vspc& color,
    float t = 0
);


} // end namespace lux
