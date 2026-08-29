#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "volume.h"
#include "Vector.h"
#include "implicit_fields.h"
#include "field_operations.h"
#include "field_interface.h"

using namespace lux;
using vspf = VolumeSPtr<float>;

TEST_CASE("Testing templated Field operations"){

    
    // Create two templated ConstField<float>
    float a_val = 7.0;
    float b_val = 5.0;
    std::shared_ptr<Volume<float>> a = std::make_shared<ConstantField<float>>(a_val);
    std::shared_ptr<Volume<float>> b = std::make_shared<ConstantField<float>>(b_val);

    VolumeSPtr<float> c = add(a, b);
    VolumeSPtr<float> d = a + b;
    REQUIRE(c->eval(Vector(3, 3, 3)) == 12);
    REQUIRE(d->eval(Vector(3, 3, 3)) == 12);

    VolumeSPtr<float> sub = a - b;
    REQUIRE(sub->eval(Vector()) == (a_val - b_val));
    
    // switching to plane to actuall test gradients
    Vector point(1, 1, 1);
    Vector normal(1, 0, 0);
    Vector point2(10, -10, 15);
    Vector normal2(-3, 2, 7);
    VolumeSPtr<float> plane1 = make_plane(point, normal);
    VolumeSPtr<float> plane2 = make_plane(point2, normal2);

    Vector eval_point(7, 7, -7);
    float p1_eval = normal * (eval_point - point);
    float p2_eval = normal2 * (eval_point - point2);
    // vspf negate = -plane1;
    // REQUIRE(negate->eval(eval_point) == -p1_eval);
    // REQUIRE(negate->grad(eval_point) == -normal); 
}


TEST_CASE("Test implicit plane"){
    // A plane has a point that is on the surface, and a normal direction
    // Positive above the field and negative below
    Vector point(1, 1, 1);
    Vector normal(1, 0, 0);
    VolumeSPtr<float> plane = make_plane(point, normal);

    REQUIRE(plane->eval(Vector(2, -1, 10)) == 1.0);
    REQUIRE(plane->eval(Vector(0, 0, 0))== -1.0);
    REQUIRE(plane->grad(Vector(1, 2, 3)) == normal);

    float const_val = 3.5;
    vspf const_field = make_constant<float>(const_val);

    vspf mult = const_field * plane;
    //grad(fg) = f *grad(g) + g * grad(f);
    Vector expected = const_val * normal;
    REQUIRE(mult->grad(Vector(1, 1, 1)) == expected);

}

TEST_CASE("Test tempalted gradients"){


}