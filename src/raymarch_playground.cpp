#include "raymarch_playground.h"
#include "openvdb_helpers.h"


#include "string_funcs.h"

namespace lux{

#define RADTODEG(radians) ((radians) * 180.0f / 3.14159265358979)
#define DEGTORAD(degrees) ((degrees * 3.14159265358979 / 180.0))

using vecsptr = std::shared_ptr<const std::vector<vspf>>;



void raymarch_things(const std::string& filename, float t){
    float cam_distance = 10;
    float scene_width = 5;
    //float near = cam_distance - scene_width / 2.0;
    //float far = near + scene_width;
    float near = 5;
    float far = 15;
    RayMarcher rm;
    float min_ds = (far - near) / 100;
    float max_ds = min_ds * 3.65;
    rm.set_min_ds(min_ds);
    rm.set_max_ds(max_ds);
    rm.set_snear(near);
    rm.set_sfar(far);
    rm.set_exticntion_coefficient(3.0);
    rm.set_Tmin(0.001);
    Camera cam;

    float radius = 1.5;
    //auto sphere = isf_sphere(Vector(), radius);
    auto torus = isf_torus(Vector(), radius, radius / 3.0, Vector(0, 0, -1));
    auto torus_b = isf_torus(Vector(), radius, radius / 3.0, Vector(0, 1, 0));
    auto torus_c = isf_torus(Vector(), radius, radius / 3.0, Vector(1, 0, 0));


    auto col = make_constant(Color(0, 0, 0, 0));
    auto torus_col = make_constant(comfy_colors::green_mountain);
    
    
    float voxel_size = 0.1;
    float default_val = voxel_size;
    auto grid = openvdb::FloatGrid::create(default_val);
    grid->setGridClass(openvdb::GridClass::GRID_FOG_VOLUME);
    grid->setTransform(openvdb::math::Transform::createLinearTransform(voxel_size));
    auto bounds = world_space_to_bounds(Vector(-radius, -radius, -radius) * 1.5, Vector(radius, radius, radius) * 1.5, grid);
    printf("bounds llc = %d, %d, %d", bounds.getStart().x(), bounds.getStart().y(), bounds.getStart().z());
    
    stamp_grid<float>(grid, [torus](const Vector& p){return torus->eval(p); }, [](float val){ return val < 0; }, bounds);
    torus = make_grid_field<float>(grid);
    
    
    // auto col_grid = openvdb::Vec3SGrid::create();
    // col_grid->setGridClass(openvdb::GridClass::GRID_FOG_VOLUME);
    // col_grid->setTransform(openvdb::math::Transform::createLinearTransform(voxel_size));
    // stamp_grid<Color>(col_grid, [col](const Vector& p){ return col->eval(p); }, [](const Color& val){return true; }, bounds);
    
    //col = make_grid_field<Color>(col_grid);

    auto gb = openvdb::FloatGrid::create(default_val);
    gb->setGridClass(openvdb::GridClass::GRID_FOG_VOLUME);
    gb->setTransform(openvdb::math::Transform::createLinearTransform(voxel_size));
    stamp_grid<float>(gb, [torus_b](const Vector& p){return torus_b->eval(p); }, [](float val){ return val < 0; }, bounds);
    torus_b = make_grid_field<float>(gb);
    col = torus_col * mask(torus) + col * mask(-torus);
    auto col_b = make_constant(comfy_colors::purpple_eastside);
    col = col_b * mask(torus_b) + col * mask(-torus_b);
    torus = union_fields(torus, torus_b);
    
    
    //torus = make_grid_field<float>(torus);

    torus = union_fields(torus, torus_c);
    auto col_c = make_constant(comfy_colors::rose);
    col = col_c * mask(torus_c) + col * mask(-torus_c);



    ImageData render_img(1920 / 4, 1080 / 4, 4);
    Vector eye = Vector(0, 0, cam_distance);
    Vector view = Vector(0, 0, -1);
    eye = rotation(eye, Vector(0, 1, 0), DEGTORAD(360.0 / 1 * 0));
    view = rotation(view, Vector(0, 1, 0), DEGTORAD(360.0 / 1 * 0));
    cam.setEyeViewUp(eye, view, Vector(0, 1, 0));
    rm.ray_march_image(cam, render_img, torus, col);
    std::string frame_filename = filename;
    render_img.oiio_write_to(frame_filename);
}

} // end namespace lux