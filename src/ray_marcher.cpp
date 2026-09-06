#include "ray_marcher.h"
#include <iostream>

using namespace lux;


// void RayMarcher::set_dimensions(int x, int y){
//     _img_data.set_dimensions(x, y, 4); // 4 for rgba
// }



Color RayMarcher::ray_march_single_pixel(
        const Vector& direction,
        const Vector& eye,
        const VolumeSPtr<float>& density,
        const VolumeSPtr<Color>& color
) const {
    double T = 1; // Transmisivity
    Color L(0,0,0,0);
    double s = _snear;
    Vector X = eye + direction * s;
    while( s < _sfar && T > _Tmin ) {
        float den = density->eval(X);
        if( den < 0.0 ){
            if (_kappa == 0.0){
                L += color->eval(X);
                T = 0;
            } else {
                float dT = std::exp( _ds * _kappa * den ); // dens is negative here, so remove - mult;
                L += color->eval(X) * (1-dT) * T * _one_over_kappa;
                T *= dT;
            }
        }
        X += direction * _ds;
        s += _ds;
    }
    L[3] = 1-T; // set the alpha channel to the opacity
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
    const float nx_pixelsf = (float)img.get_width();
    const float ny_pixelsf = (float)img.get_height();
    const Vector& rhat = cam.right();
    const Vector& vhat = cam.up();
    const Vector& ncam = cam.view();
    const Vector& eye = cam.eye();
    printf("img width = %d, img height = %d\n", img.get_width(), img.get_height());
    #pragma omp parallel for
    for (int j = 0; j < img.get_height(); j++){
        for (int i = 0; i < img.get_width(); i++){
            float u = (2.0 * i / nx_pixelsf - 1.0) * htanfov;
            float v = (2.0 * j / ny_pixelsf - 1.0) * vtanfov;

            Vector ray_dir = (u * rhat + v * vhat + ncam).unitvector();
            Color pixel = ray_march_single_pixel(ray_dir, eye, density, color);
            ImageData::pixel p = {(float)pixel.red(), (float)pixel.green(), (float)pixel.blue(), (float)pixel.alpha()};
            img.set_pixel_values(i, j, p);
            if (i == 1920 / 2 / 2 && j == 1080 / 2 / 2){
                pixel.__str__();
                printf("found\n");
                printf("%f %f %f %f\n", pixel.red(), pixel.blue(), pixel.green(), pixel.alpha());
            }
        }
    }

    
}