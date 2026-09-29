#include "openvdb_helpers.h"
#include <openvdb/tools/MeshToVolume.h>

#include "obj_reader.h" 
#include "AABB.h"

namespace lux{



openvdb::CoordBBox world_space_to_bounds(const Vector& llc, const Vector& urc, float voxel_size){
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

openvdb::CoordBBox world_space_to_bounds(const Vector& llc, const Vector& urc, const openvdb::FloatGrid::Ptr grid){
    auto voxel_size = grid->transform().voxelSize().x();
    return world_space_to_bounds(llc, urc, voxel_size);
}


openvdb::FloatGrid::Ptr obj_mesh_to_level_set_f(std::string& obj_path, float voxel_size, float half_width){
    ObjReader<openvdb::Vec3s> objreader(obj_path);
    const auto& verts = objreader.get_verts();
    const auto& faces = objreader.get_faces();
    std::vector<openvdb::Vec3s> points;
    points.reserve(verts.size());
    std::copy(verts.begin(), verts.end(), std::back_inserter(points));
    auto face_to_openvdb = [](const cato::Vec3i& face) {
        return openvdb::Vec3I(face.x(), face.y(), face.z());
    };

    std::vector<openvdb::Vec3I> tris;
    tris.reserve(faces.size());
    std::transform(faces.begin(), faces.end(), std::back_inserter(tris), face_to_openvdb);

    openvdb::Vec3s center;
    for (const auto& vert : verts){
        center += vert;
    }
    center = center / (double)verts.size();

    auto transform = openvdb::math::Transform::createLinearTransform(voxel_size);
    auto grid = openvdb::tools::meshToLevelSet<openvdb::FloatGrid>(
        *transform, points, tris, half_width
    );
    grid->transform().postTranslate(-center);
    return grid;
}

VSPtr<float> obj_mesh_to_grid_field(std::string& obj_path, float voxel_size, float half_width){
    return make_grid_field<float>(obj_mesh_to_level_set_f(obj_path, voxel_size, half_width));
}
    


} // end namespace lux