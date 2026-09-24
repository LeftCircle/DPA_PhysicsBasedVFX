#pragma once

#include <openvdb/openvdb.h>

#include "Vector.h"

namespace lux{

using ocoord = openvdb::Coord;

openvdb::CoordBBox world_space_to_bounds(const Vector& llc, const Vector& urc, const openvdb::FloatGrid::Ptr grid);

template <typename Func>
void stamp_grid(openvdb::FloatGrid::Ptr grid, Func func, openvdb::CoordBBox bounds){
    auto voxel_size = grid->transform().voxelSize().x();
    auto accessor = grid->getAccessor();
    // now loop over each voxel in the AABB
    for (int z = bounds.getStart().z(); z <= bounds.getEnd().z(); z++){
        for (int y = bounds.getStart().y(); y <= bounds.getEnd().y(); y++){
            for (int x = bounds.min().x(); x <= bounds.max().x(); x++){
                Vector p = Vector(x, y, z) * voxel_size;
                if (func(p) > 0) continue;
                accessor.setValue(ocoord(x, y, z), func(p));
            }
        }
    }
}







}



