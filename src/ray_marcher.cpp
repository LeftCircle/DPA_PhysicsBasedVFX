#include "ray_marcher.h"

using namespace lux;


void RayMarcher::set_dimensions(int x, int y){
    _img_data.set_dimensions(x, y, 3);
}