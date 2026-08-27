#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "volume.h"
#include "Volume.h"
#include "Fields.h"
#include "ImplicitVolumeShapes.h"
#include "Vector.h"
#include "implicit_fields.h"
#include "field_operations.h"

using namespace lux_bb;

TEST_CASE("Testing templated ConstField"){
    // 1.) Create two constant scalar fields
    //std::shared_ptr<lux::ConstantVolume> a = std::make_shared<lux::ConstantVolume>(7);
    //std::shared_ptr<lux::ConstantVolume> a = std::make_shared<lux::ConstantVolume>(5);
    lux::ScalarField a = SF(new lux::ConstantVolume(7));
    lux::ScalarField b = SF(new lux::ConstantVolume(5));


    // Create two templated ConstField<float>
    std::shared_ptr<ConstantField<float>> a2 = std::make_shared<ConstantField<float>>(7);
    std::shared_ptr<ConstantField<float>> b2 = std::make_shared<ConstantField<float>>(5);

    // Add fields together
    lux::AddVolume c(a, b);
    DoubleHomoField<float> c2(a2, b2);

    //DoubleHomoField<float> c3 = a2 + b2;


    // Confirm evaulation matches
    REQUIRE(c.eval(lux::Vector(3, 3, 3)) == 12);
    REQUIRE(c2.eval(lux::Vector(3, 3, 3)) == 12);
    //REQUIRE(c3.eval(lux::Vector(3, 3, 3)) == 12);
}