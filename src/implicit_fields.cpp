#include "implicit_fields.h"


using namespace lux;


const TorusField::volumeDataType TorusField::eval(const Vector& p) const {
    float term1 = (_center - p).magnitude_squared() + _r_major_sq - _r_minor_sq;
    return term1 * term1 - (4.0 * _r_major_sq * (p - (p*_n_hat) * _n_hat).magnitude_squared());
}


