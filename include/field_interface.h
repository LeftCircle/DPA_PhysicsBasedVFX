#pragma once


#include "volume.h"
#include "field_operations.h"

namespace lux{

    
template <typename T>
std::shared_ptr<Volume<T>> add(const VolumeSPtr<T>& a, const VolumeSPtr<T>& b) {
    //return std::dynamic_pointer_cast<Volume<T>>(make_shared<AddFields<T>>(a, b));
    //return std::shared_ptr<Volume<T>>(new AddFields(a, b));
    return std::make_shared<AddFields<T>>(a, b);
}

template <typename T>
std::shared_ptr<Volume<T>> operator+(const VolumeSPtr<T>& a,
                                     const VolumeSPtr<T>& b) {
    return add(a, b);
}

}



