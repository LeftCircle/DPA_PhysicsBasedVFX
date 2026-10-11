#pragma once

#include <string>
#include <limits>
#include <openvdb/openvdb.h>
#include <openvdb/tools/ValueTransformer.h>

#include "field_interface.h"
#include "Vector.h"

namespace lux{

using ocoord = openvdb::Coord;
using coordbbox = openvdb::CoordBBox;

openvdb::CoordBBox world_space_to_bounds(const Vector& llc, const Vector& urc, const openvdb::FloatGrid::Ptr grid);
openvdb::CoordBBox world_space_to_bounds(const Vector& llc, const Vector& urc, float voxel_size);
openvdb::FloatGrid::Ptr create_float_grid(float voxel_size, float default_val = 0.0f);
openvdb::Vec3SGrid::Ptr create_color_grid(float voxel_size, openvdb::Vec3s def_val = openvdb::Vec3s(0, 0, 0));

openvdb::FloatGrid::Ptr obj_mesh_to_level_set_f(std::string& obj_path, float voxel_size, float half_width);
VSPtr<float> obj_mesh_to_grid_field(std::string& obj_path, float voxel_size, float half_width);

void stamp_isf_to_grid(openvdb::FloatGrid::Ptr grid, const vspf& a, coordbbox& bounds);
vspf stamp_isf_to_grid(const vspf& a, openvdb::CoordBBox& bounds, float voxel_size, float default_value, std::string filename = "");

// partitions are the number of segments the box is broken up into. Ie partitions = 2 will break a grid into 8 segments. 
// The width, depth, and height will be split into n^3 grids, stamped in parallel, then combined.
// vspf stamp_isf_to_grid_parallel(const vspf& a, openvdb::CoordBBox& bounds, float voxel_size, float default_value, int partitions = 2);


vspc stamp_color_to_grid(const vspc& a, openvdb::CoordBBox& bounds, float voxel_size, std::string filename = "");

template <typename GridType>
void save_to_disk(const GridType& grid, std::string& filename){
    openvdb::io::File file(filename);
    file.write({grid});
    file.close();
}


openvdb::FloatGrid::Ptr mesh_to_levelset(
    float voxel_size,
    std::vector<openvdb::Vec3s> points,
    std::vector<openvdb::Vec3I> triangles,
    std::vector<openvdb::Vec4I> quads,
    float half_width
);


template <typename T, typename Func, typename CondFunc>
void stamp_grid(typename GridTypes<T>::GridType grid, Func func, CondFunc condition_func, const openvdb::CoordBBox& bounds){
    auto voxel_size = (float)grid->transform().voxelSize().x();
    //printf("Voxel size is %g \n", voxel_size);
    auto accessor = grid->getAccessor();
    // now loop over each voxel in the AABB
    for (int z = bounds.min().z(); z <= bounds.max().z(); z++){
        for (int y = bounds.min().y(); y <= bounds.max().y(); y++){
            for (int x = bounds.min().x(); x <= bounds.max().x(); x++){
                auto pvdb = grid->indexToWorld(ocoord(x, y, z));
                Vector p = Vector(pvdb.x(), pvdb.y(), pvdb.z());
                auto val = func(p);
                if (!condition_func(val)) continue;
                accessor.setValue(ocoord(x, y, z), GridTypes<T>::to_grid(val));
            }
        }
    }
}



} // end namespace lux



