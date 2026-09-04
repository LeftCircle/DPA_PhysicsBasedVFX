#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "ray_marcher.h"

using namespace lux;

TEST_CASE("test ray marcher image size"){

    RayMarcher rm;
    int test_width = 1920;
    int test_height = 1080;
    rm.set_dimensions(test_width, test_height);
    REQUIRE(rm.get_img_width() == test_width);
    REQUIRE(rm.get_img_height() == test_height);

    ImageData test_img(test_width, test_height, 3);
    test_img.oiio_write_to("../tests/test_outputs/test_image_write.exr");
    std::printf("Image should hopefully have been written out\n");
}

TEST_CASE("Test print works"){
    std::printf("There should be output\n");
    REQUIRE(false);
}