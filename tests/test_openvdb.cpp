#include <iostream>
#include <catch2/catch_test_macros.hpp>

#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h> // replace with your own dependencies for generating the OpenVDB grid
#include <nanovdb/tools/CreateNanoGrid.h> // converter from OpenVDB to NanoVDB (includes NanoVDB.h and GridManager.h)
#include <nanovdb/io/IO.h>

#include "field_interface.h"
#include "openvdb_helpers.h"

using namespace lux;

TEST_CASE("test openvdb works with hello world example"){
    // Hello world is here: https://www.openvdb.org/documentation/doxygen/codeExamples.html#sHelloWorld
        // Initialize the OpenVDB library.  This must be called at least
    // once per program and may safely be called multiple times.
    openvdb::initialize();
 
    // Create an empty floating-point grid with background value 0.
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();
 
    std::cout << "Testing random access:" << std::endl;
 
    // Get an accessor for coordinate-based access to voxels.
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();
 
    // Define a coordinate with large signed indices.
    openvdb::Coord xyz(1000, -200000000, 30000000);
 
    // Set the voxel value at (1000, -200000000, 30000000) to 1.
    accessor.setValue(xyz, 1.0);
 
    // Verify that the voxel value at (1000, -200000000, 30000000) is 1.
    std::cout << "Grid" << xyz << " = " << accessor.getValue(xyz) << std::endl;
 
    // Reset the coordinates to those of a different voxel.
    xyz.reset(1000, 200000000, -30000000);
 
    // Verify that the voxel value at (1000, 200000000, -30000000) is
    // the background value, 0.
    std::cout << "Grid" << xyz << " = " << accessor.getValue(xyz) << std::endl;
 
    // Set the voxel value at (1000, 200000000, -30000000) to 2.
    accessor.setValue(xyz, 2.0);
 
    // Set the voxels at the two extremes of the available coordinate space.
    // For 32-bit signed coordinates these are (-2147483648, -2147483648, -2147483648)
    // and (2147483647, 2147483647, 2147483647).
    accessor.setValue(openvdb::Coord::min(), 3.0f);
    accessor.setValue(openvdb::Coord::max(), 4.0f);
 
    std::cout << "Testing sequential access:" << std::endl;
 
    // Print all active ("on") voxels by means of an iterator.
    for (openvdb::FloatGrid::ValueOnCIter iter = grid->cbeginValueOn(); iter; ++iter) {
        std::cout << "Grid" << iter.getCoord() << " = " << *iter << std::endl;
    }
    REQUIRE(true);
}


TEST_CASE("test nano vdb works"){
    try {
        // Create an OpenVDB grid (here a level set surface but replace this with your own code)
        auto srcGrid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(100.0f, openvdb::Vec3f(0.0f), 1.0f);
 
        // Convert the OpenVDB grid, srcGrid, into a NanoVDB grid handle.
        auto handle = nanovdb::tools::createNanoGrid(*srcGrid);
 
        // Define a (raw) pointer to the NanoVDB grid on the host. Note we match the value type of the srcGrid!
        auto* dstGrid = handle.grid<float>();
 
        if (!dstGrid)
            throw std::runtime_error("GridHandle does not contain a grid with value type float");
 
        // Get accessors for the two grids. Note that accessors only accelerate repeated access!
        auto dstAcc = dstGrid->getAccessor();
        auto srcAcc = srcGrid->getAccessor();
 
        // Access and print out a cross-section of the narrow-band level set from the two grids
        for (int i = 97; i < 104; ++i) {
            printf("(%3i,0,0) OpenVDB cpu: % -4.2f, NanoVDB cpu: % -4.2f\n", i, srcAcc.getValue(openvdb::Coord(i, 0, 0)), dstAcc.getValue(nanovdb::Coord(i, 0, 0)));
        }
 
        nanovdb::io::writeGrid("data/sphere.nvdb", handle); // Write the NanoVDB grid to file and throw if writing fails
    }
    catch (const std::exception& e) {
        std::cerr << "An exception occurred: \"" << e.what() << "\"" << std::endl;
    }
}


TEST_CASE("Test stamping IF into a grid"){
    float radius = 2.05;
    Vector center;
    auto sphere = isf_sphere(center, radius);

    // Now create the grid
    openvdb::initialize();

    float voxel_size = 0.1;
    float half_narrow_bandwidth = voxel_size * 3;
    //auto level_set = openvdb::createLevelSet<openvdb::FloatGrid>(voxel_size, half_narrow_bandwidth);
    float default_val = 100.0;
    auto grid = openvdb::FloatGrid::create(default_val);
    grid->setTransform(openvdb::math::Transform::createLinearTransform(voxel_size));

    // Bounds should completely encompas the llc/urc
    Vector urc = Vector(1, 1, 1) * radius / 2.0;;
    Vector llc = -llc;
    openvdb::CoordBBox bounds = world_space_to_bounds(llc, urc, grid);


    int width = (int)((std::ceil((urc - llc).X() / voxel_size)));
    int height = (int)((std::ceil((urc - llc).Y() / voxel_size)));
    int depth = (int)((std::ceil((urc - llc).Z() / voxel_size)));
    width = width % 2 == 0 ? width : width + 1;
    height = height % 2 == 0 ? height : height + 1;
    depth = depth % 2 == 0 ? depth : depth + 1;

    auto expected_bounds = openvdb::CoordBBox(
        openvdb::Coord(-width / 2, -height / 2, -depth / 2),
        openvdb::Coord(width / 2, height / 2, depth / 2)
    );

    REQUIRE(expected_bounds.getStart() == bounds.getStart());
    REQUIRE(expected_bounds.getEnd() == bounds.getEnd());

    
    
    // stamp into level set
    stamp_grid(grid, isf_sphere->eval, bounds);
    
    // evaluate level set at some points
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();

    // The left most point on the x axis shouldn't have a value due to padding, but one beside it should
    REQUIRE(accessor.getValue(openvdb::Coord(-width / 2, 0, 0)) == default_val);
    REQUIRE(accessor.getValue(openvdb::Coord(-width / 2 + 1, 0, 0)) <= 0);

    REQUIRE(accessor.getValue(openvdb::Coord(0, 0, 0)) == -radius);


}