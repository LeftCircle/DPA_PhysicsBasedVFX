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

    float radius = 2.0;
    auto sphere = isf_sphere(Vector(), radius);
    auto col = make_constant(Color(0, 0, 0, 0));
    auto sphere_col = make_constant(comfy_colors::green_mountain);
    col = sphere_col * mask(sphere) + col * mask(-sphere);
    
    
    float voxel_size = 1.0;
    float default_val = voxel_size;
    auto grid = openvdb::FloatGrid::create(default_val);
    grid->setGridClass(openvdb::GridClass::GRID_FOG_VOLUME);
    grid->setTransform(openvdb::math::Transform::createLinearTransform(voxel_size));
    auto bounds = world_space_to_bounds(Vector(-radius, -radius, -radius), Vector(radius, radius, radius), grid);
    stamp_grid(grid, [sphere](const Vector& p){return sphere->eval(p); }, bounds);
    auto new_sphere = make_grid_field(grid);
    


    ImageData render_img(1920 / 4, 1080 / 4, 4);
    Vector eye = Vector(0, 0, cam_distance);
    Vector view = Vector(0, 0, -1);
    eye = rotation(eye, Vector(0, 1, 0), DEGTORAD(360.0 / 1 * 0));
    view = rotation(view, Vector(0, 1, 0), DEGTORAD(360.0 / 1 * 0));
    cam.setEyeViewUp(eye, view, Vector(0, 1, 0));
    rm.ray_march_image(cam, render_img, new_sphere, col);
    std::string frame_filename = filename;
    render_img.oiio_write_to(frame_filename);
}

} // end namespace lux