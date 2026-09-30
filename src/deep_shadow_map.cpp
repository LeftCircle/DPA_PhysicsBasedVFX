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
    auto eval_func = [density_field, step_size, kappa](const Vector& p){ 
        return density_field->eval(p) * step_size * kappa;
    };
    auto accessor = grid->getAccessor();
    int steps = 0;
    int writes = 0;
    int density_hits = 0;
    for (auto iter = bounds.beginXYZ(); iter != bounds.endXYZ(); ++iter){
        steps += 1;
        Vector start_vec(grid->indexToWorld(*iter));
        if (eval_func(start_vec) >= 0) continue;
        density_hits += 1;
        auto val = accumulate_over_steps(grid->indexToWorld(*iter), light.position, step_size, eval_func);
        accessor.setValue(*iter, val);
        // auto end = Vector(grid->indexToWorld(*iter));

        // const auto dir = (light.position - end).unitvector();
        // float smax = (light.position - end).magnitude();
        // float s = 0;
        // float T = 1;
        // float t_exponent = 0.0;
        // bool write_val = true;
        // while (s < smax){
        //     auto pos = end + s * dir;
        //     t_exponent += eval_func(pos);
        //     s += step_size;
        //     // if (std::expf(t_exponent) < 0.001){
        //     //     write_val = false;
        //     //     break;
        //     // }
        // }
        // //if (write_val){
        // accessor.setValue(*iter, t_exponent);
        // writes += 1;
        //}
    }

    printf("Steps %i writes %i density hits %i\n", steps, writes, density_hits);
    // transmissivty of shadowgrid
    vspf TL = exp(make_grid_field<float>(grid));
    return TL;
}



} // end namespace lux

