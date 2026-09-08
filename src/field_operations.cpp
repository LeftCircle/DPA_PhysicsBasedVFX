#include "field_operations.h"

using namespace lux;

// template <typename T>
// std::shared_ptr<Volume<T>> Volume<T>::operator+(const std::shared_ptr<Volume<T>>& e2 ) {
//     return std::make_shared<AddFields<T>>(*this, e2);
// }

// const FunctionField::volumeGradType FunctionField::grad(const Vector& p) const {
//     if (_grad_func){
//         return _grad_func(p);
//     } else {
//         return Volume<float>::grad(p);
//     }
// }