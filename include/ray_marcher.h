#pragma once

#include <Vector.h>

#include "image_data.h"
#include "volume.h"
#include "Color.h"
#include "Camera.h"
#include "deep_shadow_map.h"
#include "raytracer.h"

namespace lux{

struct RayHitInfo{
    RayHitInfo(const Vector& hit, const Vector& dir, int x, int y) : 
        hit_pos(hit), ray_dir(dir) { pixel[0] = x; pixel[1] = y; }
    Vector hit_pos;
    Vector ray_dir;
    int pixel[2];
};


class RayMarcher{
public:
    Color ray_march_single_pixel(
        const Vector& direction,
        const Vector& eye,
        const VolumeSPtr<float>& density,
        const VolumeSPtr<Color>& color
    ) const;

    Color ray_march_single_pixel(
        const Vector& start_pos,
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

    template <typename Vec, typename Vec3i, typename Vec4i>
    void ray_march_image(
        Camera cam, 
        ImageData& img,
        const vspf& density,
        const VSPtr<Color>& color,
        const std::vector<Vec3i>& tris,
        const std::vector<Vec4i>& faces,
        const std::vector<Vec>& verts
    ){
        // First let's find the pixel size. Can we assume square pixels??
        const float htanfov = cam.get_htanfov();
        const float vtanfov = cam.get_vtanfov();
        const float one_over_nx_pixelsf = 1.0 / (float)img.get_width();
        const float one_over_ny_pixelsf = 1.0 / (float)img.get_height();
        const Vector& rhat = cam.right();
        const Vector& vhat = cam.up();
        const Vector& ncam = cam.view();
        const Vector& eye = cam.eye();

        // Now let's find all pixels that intersect
        std::vector<RayHitInfo> raytrace_info;
        for (int j = 0; j < img.get_height(); j++){
            for (int i = 0; i < img.get_width(); i++){
                float u = (2.0 * i * one_over_nx_pixelsf - 1.0) * htanfov;
                float v = (2.0 * j * one_over_ny_pixelsf - 1.0) * vtanfov;

                Vector ray_dir = (u * rhat + v * vhat + ncam).unitvector();
                auto hit = get_first_hit_position(eye, ray_dir, tris, faces, verts);
                if (hit){
                    raytrace_info.push_back(RayHitInfo(*hit, ray_dir, i, j));
                }
            }
        }

        #pragma omp parallel for
        for (const auto& rayinfo : raytrace_info){
            Color pixel = ray_march_single_pixel(rayinfo.hit_pos, rayinfo.ray_dir, eye, density, color);
            ImageData::pixel p = {(float)pixel.red(), (float)pixel.green(), (float)pixel.blue(), (float)pixel.alpha()};
            img.set_pixel_values(rayinfo.pixel[0], rayinfo.pixel[1], p);
        }
    }

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


