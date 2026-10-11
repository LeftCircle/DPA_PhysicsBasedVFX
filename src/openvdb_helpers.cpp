#include "openvdb_helpers.h"
#include <openvdb/tools/MeshToVolume.h>
#include <tbb/parallel_for.h>

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

openvdb::FloatGrid::Ptr create_float_grid(float voxel_size, float default_val){
    auto grid = openvdb::FloatGrid::create(default_val);
    grid->setGridClass(openvdb::GridClass::GRID_FOG_VOLUME);
    grid->setTransform(openvdb::math::Transform::createLinearTransform(voxel_size));
    return grid;
}

// TODO this should be templated
openvdb::Vec3SGrid::Ptr create_color_grid(float voxel_size, openvdb::Vec3s def_val){
    auto grid = openvdb::Vec3SGrid::create(def_val);
    grid->setGridClass(openvdb::GridClass::GRID_FOG_VOLUME);
    grid->setTransform(openvdb::math::Transform::createLinearTransform(voxel_size));
    return grid;
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


openvdb::FloatGrid::Ptr mesh_to_levelset(
    float voxel_size,
    std::vector<openvdb::Vec3s> points,
    std::vector<openvdb::Vec3I> triangles,
    std::vector<openvdb::Vec4I> quads,
    float half_width
){
    //std::vector<openvdb::Vec3I> quad_tris;
    triangles.reserve(triangles.size() + 2 * quads.size());
    
    for (const auto& quad : quads) {
        triangles.emplace_back(quad[0], quad[1], quad[2]);
        triangles.emplace_back(quad[0], quad[2], quad[3]);
    }
    //triangles.insert(triangles.end(), quad_tris.begin(), quad_tris.end());
    auto transform = openvdb::math::Transform::createLinearTransform(voxel_size);
    return openvdb::tools::meshToLevelSet<openvdb::FloatGrid>(*transform, points, triangles, half_width);
}


// vspf stamp_isf_to_grid(const vspf& a, openvdb::CoordBBox& bounds, float voxel_size, float default_value){
//     auto g = create_float_grid(voxel_size, default_value);
//     stamp_isf_to_grid(g, a, bounds);
//     return make_grid_field<float>(g);
// }

vspf stamp_isf_to_grid(
    const vspf& a,
    openvdb::CoordBBox& bounds,
    float voxel_size,
    float default_value,
    std::string filename
){

    // start by creating an array of bounds
    //std::vector<openvdb::FloatGrid::Ptr> local_grids(partitions);
    auto func = [a](const Vector& p){ return a->eval(p); };
    auto cond = [](const float val){ return val < 0; };
    
    // let tbb craete some local grids
    tbb::enumerable_thread_specific<openvdb::FloatGrid::Ptr> thread_grids(
        [&](){return create_float_grid(voxel_size, default_value); }
    );
    tbb::parallel_for(
        bounds,
        [&](const openvdb::CoordBBox& sub_bounds){
            auto local_grid = thread_grids.local();
            stamp_grid<float>(
                local_grid,
                func,
                cond,
                sub_bounds
            );
        }
    );
    
    auto result = create_float_grid(voxel_size, default_value);

    for (const auto& local_grid : thread_grids) {
        result->tree().merge(local_grid->tree());
    }

    if (filename != ""){
        save_to_disk(result, filename);
    }

    return make_grid_field<float>(result);
}

void stamp_isf_to_grid(openvdb::FloatGrid::Ptr grid, const vspf& a, coordbbox& bounds){
    auto func = [a](const Vector& p){ return a->eval(p); };
    auto cond = [](const float val){ return val <= 0; };
    stamp_grid<float>(grid, func, cond, bounds);
}

vspc stamp_color_to_grid(
    const vspc& col,
    openvdb::CoordBBox& bounds,
    float voxel_size,
    std::string filename

){
    auto func = [col](const Vector& p){ return col->eval(p); };
    auto cond = [](const Color& val){return true; };
    
    // let tbb craete some local grids
    tbb::enumerable_thread_specific<openvdb::Vec3SGrid::Ptr> thread_grids(
        [&](){return create_color_grid(voxel_size); }
    );
    tbb::parallel_for(
        bounds,
        [&](const openvdb::CoordBBox& sub_bounds){
            auto local_grid = thread_grids.local();
            stamp_grid<Color>(
                local_grid,
                func,
                cond,
                sub_bounds
            );
        }
    );
    
    auto result = create_color_grid(voxel_size);

    for (const auto& local_grid : thread_grids) {
        result->tree().merge(local_grid->tree());
    }

    if (filename != ""){
        save_to_disk(result, filename);
    }
    return make_grid_field<Color>(result);
    
    // auto col_grid = openvdb::Vec3SGrid::create();
    // col_grid->setGridClass(openvdb::GridClass::GRID_FOG_VOLUME);
    // col_grid->setTransform(openvdb::math::Transform::createLinearTransform(voxel_size));
    
    // stamp_grid<Color>(col_grid, [col](const Vector& p){ return col->eval(p); }, [](const Color& val){return true; }, bounds);

    // return make_grid_field<Color>(col_grid);

}


} // end namespace lux