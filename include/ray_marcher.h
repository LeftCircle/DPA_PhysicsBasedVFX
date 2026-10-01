#pragma once

#include "image_data.h"
#include "volume.h"
#include "Color.h"
#include "Camera.h"
#include "deep_shadow_map.h"

namespace lux{

class RayMarcher{
public:
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

    void ray_march_image(
        Camera cam, 
        ImageData& img_data,
        const vspf& density,
        const VSPtr<Color>& color,
        const float other
    );

    void set_snear(float snear) { _snear = snear; }
    void set_sfar(float sfar) { _sfar = sfar; }
    void set_Tmin(float tmin) { _Tmin = tmin; }
    void set_ds(float ds) { _ds = ds; }
    void set_min_ds(float min) { _min_ds = min; }
    void set_max_ds(float max) { _max_ds = max; }
    void set_exticntion_coefficient(float kappa) { _kappa = kappa; _one_over_kappa = 1.0 / kappa; }

    void add_shadow_map(vspf shadow_map, Color light_col) { 
        _shadow_maps.push_back(shadow_map);
        _light_colors.push_back(light_col);
    }

private:
    //ImageData _img_data;
    float _snear;
    float _sfar;
    float _Tmin = 0.01;
    float _ds = 1.0;
    float _kappa = 1.0; // extinction coefficient
    float _one_over_kappa = 1.0;
    float _min_ds;
    float _max_ds;

    std::vector<vspf> _shadow_maps;
    std::vector<Color> _light_colors;
};

}


