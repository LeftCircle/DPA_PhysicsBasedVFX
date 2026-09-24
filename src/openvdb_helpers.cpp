#include "openvdb_helpers.h"


namespace lux{


openvdb::CoordBBox world_space_to_bounds(const Vector& llc, const Vector& urc, const openvdb::FloatGrid::Ptr grid){
    auto voxel_size = grid->transform().voxelSize().x();
    //auto voxel_size = vs;
    int width = (int)((std::ceil((urc - llc).X() / voxel_size)));
    int height = (int)((std::ceil((urc - llc).Y() / voxel_size)));
    int depth = (int)((std::ceil((urc - llc).Z() / voxel_size)));
    width = width % 2 == 0 ? width : width + 1;
    height = height % 2 == 0 ? height : height + 1;
    depth = depth % 2 == 0 ? depth : depth + 1;

    return openvdb::CoordBBox(
            openvdb::Coord(-width / 2, -height / 2, -depth / 2),
            openvdb::Coord(width / 2, height / 2, depth / 2)
        );
}
 
    


} // end namespace lux