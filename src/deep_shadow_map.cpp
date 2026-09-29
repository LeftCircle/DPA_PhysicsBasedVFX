#include "deep_shadow_map.h"



namespace lux{



vspf make_deep_shadow_map(
    vspf density_field,
    float voxel_size,
    coordbbox bounds,
    PointLight light,
    float step_size,
    float kappa
){
    // create the floatgrid based on the inputs
    auto grid = create_float_grid(voxel_size);

    // loop over each bound, calculate density to the point light. 
    //auto end_world = grid->worldToIndex(light.position);
    auto eval_func = [density_field](const Vector& p){ return density_field->eval(p); };
    auto accessor = grid->getAccessor();
    for (auto iter = bounds.beginXYZ(); iter < bounds.endXYZ(); ++iter){
        Vector start_vec(grid->indexToWorld(*iter));
        auto val = accumulate_over_steps(grid->indexToWorld(*iter), light.position, step_size, eval_func);
        accessor.setValue(*iter, val * kappa); // our dens is negative so remove - mult on kappa
    }

    // transmissivty of shadowgrid
    vspf TL = exp(make_grid_field<float>(grid));
    return TL;
}



} // end namespace lux

