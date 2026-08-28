#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "volume.h"
#include "Vector.h"
#include "implicit_fields.h"
#include "field_operations.h"
#include "field_interface.h"

using namespace lux;

TEST_CASE("Testing templated ConstField"){
    // 1.) Create two constant scalar fields
    //std::shared_ptr<lux::ConstantVolume> a = std::make_shared<lux::ConstantVolume>(7);
    //std::shared_ptr<lux::ConstantVolume> a = std::make_shared<lux::ConstantVolume>(5);
    // lux::ScalarField a = SF(new lux::ConstantVolume(7));
    // lux::ScalarField b = SF(new lux::ConstantVolume(5));



    // Create two templated ConstField<float>
    std::shared_ptr<Volume<float>> a = std::make_shared<ConstantField<float>>(7);
    std::shared_ptr<Volume<float>> b = std::make_shared<ConstantField<float>>(5);

    VolumeSPtr<float> c = add(a, b);
    VolumeSPtr<float> d = a + b;
    REQUIRE(c->eval(Vector(3, 3, 3)) == 12);
    REQUIRE(d->eval(Vector(3, 3, 3)) == 12);
    

    // Add fields together
    //lux::AddVolume c(a, b);
    // AddFields<float> c2(a2, b2);

//     lux::ScalarField d = a + b;
//     std::shared_ptr<Volume<float>> d2 = a2 + b2;


//     // Confirm evaulation matches
//     REQUIRE(c.eval(lux::Vector(3, 3, 3)) == 12);
//     REQUIRE(c2.eval(lux::Vector(3, 3, 3)) == 12);
//     REQUIRE(d->eval(lux::Vector(3, 3, 3)) == 12);
//     REQUIRE(d2.eval(lux::Vector(3, 3, 3)) == 12);

//     REQUIRE(true);
}