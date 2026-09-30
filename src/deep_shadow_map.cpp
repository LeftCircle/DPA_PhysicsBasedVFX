#include "deep_shadow_map.h"



namespace lux{



vspf make_deep_shadow_map(
    vspf density_field,
    float voxel_size,
    const coordbbox& bounds,
    const PointLight& light,
    float step_size,
    float kappa
){
    // create the floatgrid based on the inputs
    auto grid = create_float_grid(voxel_size, 0.0f);

    // loop over each bound, calculate density to the point light. 
    //auto end_world = grid->worldToIndex(light.position);
    auto eval_func = [density_field, step_size](const Vector& p){ return std::min(density_field->eval(p), 0.0f) * step_size; };
    auto accessor = grid->getAccessor();
    for (auto iter = bounds.beginXYZ(); iter < bounds.endXYZ(); ++iter){
        Vector start_vec(grid->indexToWorld(*iter));
        if (eval_func(start_vec) >= 0) continue;
        //auto val = std::clamp(accumulate_over_steps(grid->indexToWorld(*iter), light.position, step_size, eval_func), 0.0f, 1.0f);
        auto val = accumulate_over_steps(grid->indexToWorld(*iter), light.position, step_size, eval_func);
        
        accessor.setValue(*iter, val * kappa); // our dens is negative so remove - mult on kappa
    }

    // transmissivty of shadowgrid
    vspf TL = exp(make_grid_field<float>(grid));
    return TL;
}



} // end namespace lux

