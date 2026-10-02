#include "ray_marcher.h"

#include <iostream>
#include <random>

using namespace lux;


// void RayMarcher::set_dimensions(int x, int y){
//     _img_data.set_dimensions(x, y, 4); // 4 for rgba
// }

Color RayMarcher::ray_march_single_pixel(
    const Vector& start_pos,
    const Vector& direction,
    const Vector& eye,
    const VolumeSPtr<float>& density,
    const VolumeSPtr<Color>& color
) const {
    double T = 1; // Transmisivity
    Color L(0,0,0,0);
    // set s to be the distance from eye to start_pos;
    double s = (start_pos - eye).magnitude();
    Vector X = start_pos;
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<float> distribution(_min_ds, _max_ds);
    while( s < _sfar && T > _Tmin ) {
        float den = density->eval(X);
        float ds = distribution(generator);
        if( den < 0.0 ){
            float shadow_eval = 1.0;
            if (_kappa == 0.0){
                L += color->eval(X);
                T = 0;
            } else {
                Color clights(0, 0, 0, 0);
                for (int i = 0; i < _shadow_maps.size(); i++){
                    clights += _light_colors[i] * _shadow_maps[i]->eval(X);
                }
                float dT = std::exp( ds * _kappa * den ); // dens is negative here, so remove - mult;
                L += color->eval(X) * (1-dT) * T * _one_over_kappa * clights;
                //L += (1-dT) * T * _one_over_kappa * clights;
                T *= dT;
            }
        }
        X += direction * ds;
        s += ds;
    }
    L[3] = 1-T; // set the alpha channel to the opacity
    return L;
}

Color RayMarcher::ray_march_single_pixel(
        const Vector& direction,
        const Vector& eye,
        const VolumeSPtr<float>& density,
        const VolumeSPtr<Color>& color
) const {
    const Vector start_pos = eye + direction * _snear;
    return ray_march_single_pixel(start_pos, direction, eye, density, color);
}

Color RayMarcher::ray_march_single_pixel(
        const Vector& direction,
        const Vector& eye,
        const VolumeSPtr<float>& density,
        const VolumeSPtr<Color>& color,
        const openvdb::FloatGrid::Ptr& level_set
) const {
    const Vector start_pos = eye + direction * _snear;
    double T = 1; // Transmisivity
    Color L(0,0,0,0);
    // set s to be the distance from eye to start_pos;
    double s = (start_pos - eye).magnitude();
    Vector X = start_pos;
    auto vdb_x = level_set->transform().worldToIndex({X.x(), X.y(), X.z()});
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<float> distribution(_min_ds, _max_ds);

    auto hw = level_set->background();
    auto vs = level_set->voxelSize();
    auto accessor = level_set->getConstAccessor();
    float out = 0;
    float in = 0;
    
    while( s < _sfar && T > _Tmin ) {
        // evaluate the level_set. if default positive value then take big steps
        float den = density->eval(X);
        //if (den >= 0.0 && std::abs(openvdb::tools::BoxSampler::sample(level_set->tree(), vdb_x) - hw) < 1e-5){
        if (den >= 0.0 && std::abs(openvdb::tools::PointSampler::sample(level_set->tree(), vdb_x) - hw) < 1e-5){
            // take a big step
            X += direction * hw;
            vdb_x = level_set->transform().worldToIndex({X.x(), X.y(), X.z()});
            s += hw;
            out += hw;
            continue;
        }
        float ds = distribution(generator);
        if( den < 0.0 ){
            float shadow_eval = 1.0;
            if (_kappa == 0.0){
                L += color->eval(X);
                T = 0;
            } else {
                Color clights(0, 0, 0, 0);
                for (int i = 0; i < _shadow_maps.size(); i++){
                    clights += _light_colors[i] * _shadow_maps[i]->eval(X);
                }
                float dT = std::exp( ds * _kappa * den ); // dens is negative here, so remove - mult;
                L += color->eval(X) * (1-dT) * T * _one_over_kappa * clights;
                //L += (1-dT) * T * _one_over_kappa * clights;
                T *= dT;
            }
        }
        X += direction * ds;
        s += ds;
        vdb_x = level_set->transform().worldToIndex({X.x(), X.y(), X.z()});
        in += ds;
    }
    L[3] = 1-T; // set the alpha channel to the opacity
    //printf("out = %g, in = %g, out percent = %g\n", out, in, (float)out / (float)(out + in));
    return L;
}


void RayMarcher::ray_march_image(
    Camera cam,
    ImageData& img,
    const VolumeSPtr<float>& density,
    const VolumeSPtr<Color>& color
) const {
    // First let's find the pixel size. Can we assume square pixels??
    const float htanfov = cam.get_htanfov();
    const float vtanfov = cam.get_vtanfov();
    const float one_over_nx_pixelsf = 1.0 / (float)img.get_width();
    const float one_over_ny_pixelsf = 1.0 / (float)img.get_height();
    const Vector& rhat = cam.right();
    const Vector& vhat = cam.up();
    const Vector& ncam = cam.view();
    const Vector& eye = cam.eye();

    #pragma omp parallel for
    for (int j = 0; j < img.get_height(); j++){
        for (int i = 0; i < img.get_width(); i++){
            float u = (2.0 * i * one_over_nx_pixelsf - 1.0) * htanfov;
            float v = (2.0 * j * one_over_ny_pixelsf - 1.0) * vtanfov;

            Vector ray_dir = (u * rhat + v * vhat + ncam).unitvector();
            Color pixel = ray_march_single_pixel(ray_dir, eye, density, color, _levelset);
            ImageData::pixel p = {(float)pixel.red(), (float)pixel.green(), (float)pixel.blue(), (float)pixel.alpha()};
            img.set_pixel_values(i, j, p);
        }
    }
}