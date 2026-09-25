#pragma once

#include <openvdb/openvdb.h>

#include "field_interface.h"
#include "Vector.h"

namespace lux{

using ocoord = openvdb::Coord;

openvdb::CoordBBox world_space_to_bounds(const Vector& llc, const Vector& urc, const openvdb::FloatGrid::Ptr grid);

template <typename T, typename Func, typename CondFunc>
void stamp_grid(typename GridTypes<T>::GridType grid, Func func, CondFunc condition_func, openvdb::CoordBBox bounds){
    auto voxel_size = (float)grid->transform().voxelSize().x();
    printf("Voxel size is %g \n", voxel_size);
    auto accessor = grid->getAccessor();
    // now loop over each voxel in the AABB
    for (int z = bounds.min().z(); z <= bounds.max().z(); z++){
        for (int y = bounds.min().y(); y <= bounds.max().y(); y++){
            for (int x = bounds.min().x(); x <= bounds.max().x(); x++){
                //Vector p = Vector(x, y, z) * voxel_size;
                auto pvdb = grid->indexToWorld(ocoord(x, y, z));
                Vector p = Vector(pvdb.x(), pvdb.y(), pvdb.z());
                //printf("p is %g %g %g  pvdb is %g %g %g\n", p.X(), p.Y(), p.Z(), pvdb.x(), pvdb.y(), pvdb.z());
                auto val = func(p);
                if (!condition_func(val)) continue;
                //printf("AHHHHH");
                // printf("Val is %g\n", val);
                accessor.setValue(ocoord(x, y, z), GridTypes<T>::to_grid(val));
            }
        }
    }
}







}



