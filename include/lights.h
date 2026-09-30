#pragma once

#include "Color.h"


namespace lux{

struct PointLight{
    PointLight(Color _color, Vector _pos) : color(_color), position(_pos) {}
    Color color;
    Vector position;
};




} // end namespcae lux