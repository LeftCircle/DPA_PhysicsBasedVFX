#include "deep_shadow_map.h"
#include <openvdb/tools/ValueTransformer.h>


namespace lux{



vspf make_deep_shadow_map(
    vspf density_field,
    float voxel_size,
    const coordbbox& bounds,
    const PointLight& light,
    float step_size,
    float kappa
){
    return make_deep_shadow_map_parallel(density_field, voxel_size, bounds, light, step_size, kappa);
}

vspf make_deep_shadow_map_parallel(
    const vspf density_field,
    float voxel_size,
    const coordbbox& bounds,
    const PointLight& light,
    float step_size,
    float kappa
){
    // create the floatgrid based on the inputs
    auto grid = create_float_grid(voxel_size, -10000.0f);

    auto stamp_func = [density_field](const Vector& p){ return density_field->eval(p); };
    auto cond = [](const float val){ return val < 0; };
    stamp_grid<float>(grid, stamp_func, cond, bounds);

    // Grid should now have active cells can do a parallel for each now
    auto eval_func = [density_field, step_size, kappa](const Vector& p){ 
        auto val = density_field->eval(p) * step_size * kappa;
        return val < 0 ? val : 0;
    };

    auto op = [&grid, &density_field, eval_func, &light, &step_size](
        const openvdb::FloatGrid::ValueOnIter& iter
    ){
        Vector start_vec(grid->indexToWorld(iter.getCoord()));
        auto eval = density_field->eval(start_vec); // Grid _should_ only have values with density
        auto val = accumulate_over_steps(start_vec, light.position, step_size, eval_func);
        iter.setValue(val);
    };

    // Now run the computation
    openvdb::tools::foreach(grid->beginValueOn(), op);

    // transmissivty of shadowgrid
    vspf TL = exp(make_grid_field<float>(grid));
    return TL;
}



} // end namespace lux

