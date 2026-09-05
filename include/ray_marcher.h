#pragma once

#include "image_data.h"
#include "volume.h"
#include "Color.h"
#include "Camera.h"

namespace lux{

class RayMarcher{
public:

    // RayMarcher(int w, int h) { set_dimensions(w, h); }
    //void set_dimensions(int w, int h);
    
    //int get_img_width() { return _img_data.get_width(); };
    //int get_img_height() { return _img_data.get_height(); };

    Color ray_march_single_pixel(
        const Vector& direction,
        const Vector& eye,
        const VolumeSPtr<float>& density,
        const VolumeSPtr<Color>& color
    ) const;

    void ray_march_image(
        Camera cam,
        ImageData& img_data,
        const VolumeSPtr<float>& density,
        const VolumeSPtr<Color>& color
    ) const;

    void set_snear(float snear) { _snear = snear; }
    void set_sfar(float sfar) { _sfar = sfar; }
    void set_Tmin(float tmin) { _Tmin = tmin; }
    void set_ds(float ds) { _ds = ds; }
    void set_exticntion_coefficient(float kappa) { _kappa = kappa; _one_over_kappa = 1.0 / kappa; }


private:
    //ImageData _img_data;
    float _snear;
    float _sfar;
    float _Tmin = 0.01;
    float _ds = 1.0;
    float _kappa = 1.0; // extinction coefficient
    float _one_over_kappa = 1.0;
};

}


