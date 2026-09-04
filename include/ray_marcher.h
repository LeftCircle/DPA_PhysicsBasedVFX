#pragma once

#include "image_data.h"


namespace lux{

class RayMarcher{
public:

    void set_dimensions(int w, int h);
    
    int get_img_width() { return _img_data.get_width(); };
    int get_img_height() { return _img_data.get_height(); };


private:
    ImageData _img_data;
};

}


