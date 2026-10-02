#include "level_set_renders.h"

namespace lux{


void render_bunny(const std::string& path, const std::string& name){
    float voxel_size = 0.001;
    float half_width = 3;
    std::string obj_filepath = "/home/left/programming/DPA_PhysicsBasedVFX/third_party/starter/models/bunny_fixed.obj";
    ObjReader<Vector> reader(obj_filepath);
    
    printf("n tris = %g", (float)reader.get_faces().size());
    std::cout << obj_filepath<< std::endl;
    
    auto bunny2 = obj_mesh_to_level_set_f(obj_filepath, voxel_size, half_width);
    bunny2->transform().postScale(30);
    auto bunny_bounds = bunny2->evalActiveVoxelBoundingBox();
    auto bbs = bunny_bounds.getStart();
    auto bbe = bunny_bounds.getEnd();
    printf("bunny_bounds llc %i %i %i urc %i %i %i\n", bbs.x(), bbs.y(), bbs.z(), bbe.x(), bbs.y(), bbs.z());
    auto bunny = make_grid_field<float>(bunny2);
    //bunny = clamp_fixed(bunny, -1, 1);
    bunny = bunny * make_constant(100.0f);
    render_turnable(path, name, bunny, -mask(bunny), make_constant(Color(1, 1, 1, 0)));
}

void render_turnable(
    const std::string& path,
    const std::string& name,
    const vspf& density, 
    const vspf& masked_density,
    const vspc& color,
    float t
){
    // RENDER SETTINGS
    float cam_distance = 10;
    float near = 5;
    float far = 15;
    float model_halfwidth = 3;
    
    float min_ds = (far - near) / 10000;
    float max_ds = min_ds * 3.65;
    float kappa = 1.0;

    // RENDER OPTIMIZATION SETTINGS
    float mesh_grid_voxelsize = 0.1;
    float sdf_voxel_size = 0.1;
    float sdf_half_width = 3;

    // SHADOW MAP SETTINGS
    float shadow_map_voxel_size = 0.025;
    float sm_kappa = 3.5;
    float sm_stepsize = 0.0025;

    RayMarcher rm;
    
    rm.set_min_ds(min_ds);
    rm.set_max_ds(max_ds);
    rm.set_snear(near);
    rm.set_sfar(far);
    rm.set_exticntion_coefficient(kappa);
    rm.set_Tmin(0.001);
    Camera cam;


    // ------------------------------------------------------------------
    // Adding the level set to the raymarcher
    // ------------------------------------------------------------------
    float default_val = 999;
    auto grid = create_float_grid(mesh_grid_voxelsize, default_val);
    float mhw = model_halfwidth;
    auto model_bounds = world_space_to_bounds(Vector(-mhw, -mhw, -mhw), Vector(mhw, mhw, mhw), grid);
    stamp_isf_to_grid(grid, dilation(density, mesh_grid_voxelsize / 2.0), model_bounds);

    std::vector<openvdb::Vec3s> points;
    std::vector<openvdb::Vec3I> triangles;
    std::vector<openvdb::Vec4I> quads;

    openvdb::tools::volumeToMesh(*grid, points, triangles, quads);

    auto transform = openvdb::math::Transform::createLinearTransform(sdf_voxel_size);
    float hw = sdf_half_width;
    printf("Making level set\n");
    auto levelset = openvdb::tools::meshToLevelSet<openvdb::FloatGrid>(*transform, points, triangles, quads, hw);
    auto levelset_bounds = levelset->evalActiveVoxelBoundingBox();
    printf("Level set made\n");
    rm.add_levelset(levelset);

    
    // ------------------------------------------------------------------
    // creating lights
    // ------------------------------------------------------------------

    auto const_col = make_constant(Color(1, 1, 1, 0));


    // TO DO -> better to add colors to the rm and have it create the shadow map (maybe)
    PointLight key(Color(1.0, 0.00, 0.00, 0), Vector(0, 3, 3));
    PointLight fill(Color(0.00, 1.0, 0.00, 0), Vector(0, -3, 0));
    PointLight rim(Color(0.00, 0.00, 1.0, 0), Vector(0, 0, -3));

    float sm_voxelsize = shadow_map_voxel_size;
    
    //auto bounds = world_space_to_bounds({-mhw, -mhw, -mhw}, {mhw, mhw, mhw}, sm_voxelsize);
    auto bounds = world_space_to_bounds(
        Vector(levelset->indexToWorld(levelset_bounds.getStart())),
        Vector(levelset->indexToWorld(levelset_bounds.getEnd())),
        sm_voxelsize
    );
    bounds.expand(8);
    
    
    auto shaddow_map = make_deep_shadow_map_parallel(density, sm_voxelsize, bounds, key, sm_stepsize, sm_kappa);
    auto sm2 = make_deep_shadow_map(density, sm_voxelsize, bounds, fill, sm_stepsize, sm_kappa);
    auto sm3 = make_deep_shadow_map(density, sm_voxelsize, bounds, rim, sm_stepsize, sm_kappa);
    rm.add_shadow_map(shaddow_map, key.color);
    rm.add_shadow_map(sm2, fill.color);
    rm.add_shadow_map(sm3, rim.color);

    // A constant shadow map for testing if needed
    // rm.add_shadow_map(make_constant<float>(1.0f), Color(1, 0, 0, 0));
    
    //density = -mask(density);
    // ------------------------------------------------------------------
    // Rendering the image
    // ------------------------------------------------------------------
    ImageData render_img(1920, 1080, 4);
    Vector eye = Vector(0, 0, cam_distance);
    Vector view = Vector(0, 0, -1);
    eye = rotation(eye, Vector(0, 1, 0), DEGTORAD(360.0 / 1 * 0));
    view = rotation(view, Vector(0, 1, 0), DEGTORAD(360.0 / 1 * 0));
    cam.setEyeViewUp(eye, view, Vector(0, 1, 0));
    
    auto start_time = std::chrono::high_resolution_clock::now();
    rm.ray_march_image(cam, render_img, masked_density, color);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << "Raymarching took " << elapsed.count() << " seconds\n";
    
    std::string frame_filename = path + name + ".exr";
    render_img.oiio_write_to(frame_filename);
    std::cout << "Wrote to " << frame_filename << std::endl;
}


} // end namespace lux