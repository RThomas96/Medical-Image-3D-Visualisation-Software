#include <catch2/catch_test_macros.hpp>
#include "../grid/include/grid.hpp"

TEST_CASE("DivideResolution", "[grid]") {

    glm::vec3 nb = glm::vec3(5., 5., 5.);
    SimpleGrid grid("../../tests/data/img1.tif", nb, 4);

	std::vector<std::uint16_t> slices;

    int offsetOnZ = static_cast<int>(grid.grid.resolutionRatio[2]);
    int imgSizeZ = grid.grid.getImageDimensions()[2];

	//for (std::size_t s = 0; s < imgSizeZ; s+=offsetOnZ) {

    // Get first slice
    grid.grid.getGridSlice(0, slices, 1);
    CHECK(slices[309] == 9440);//x = 1236/4 = 309
    CHECK(slices[1233+309] == 6574);// x size = 1233

    slices.clear();

    grid.grid.getGridSlice(0+offsetOnZ, slices, 1);
    CHECK(slices[309] == 5098);//x = 1236
    CHECK(slices[1233+309] == 3965);
}

TEST_CASE("DivideResolutionGetPoint", "[grid]") {

    glm::vec3 nb = glm::vec3(5., 5., 5.);
    SimpleGrid grid("../../tests/data/img1.tif", nb, 4);

    int offsetOnZ = static_cast<int>(grid.grid.resolutionRatio[2]);
    int imgSizeZ = grid.grid.getImageDimensions()[2];

    uint16_t value = grid.getValueFromPoint(glm::vec3(309., 0., 0.), ResolutionMode::SAMPLER_RESOLUTION);
    CHECK(value == 9440);//x = 1236/4 = 309

    value = grid.getValueFromPoint(glm::vec3(1236., 0., 0.), ResolutionMode::FULL_RESOLUTION);
    CHECK(value == 9440);//x = 1236/4 = 309

    value = grid.getValueFromPoint(glm::vec3(309., 1., 0.), ResolutionMode::SAMPLER_RESOLUTION);
    CHECK(value == 6574);//x = 1236/4 = 309

    value = grid.getValueFromPoint(glm::vec3(1236., 4., 0.), ResolutionMode::FULL_RESOLUTION);
    CHECK(value == 6574);//x = 1236/4 = 309

    //CHECK(slices[1233+309] == 6574);// x size = 1233

    //CHECK(slices[309] == 5098);//x = 1236
    //CHECK(slices[1233+309] == 3965);
}

TEST_CASE("PointQuery", "[grid]") {
    //glm::vec3 origin = glm::vec3(0., 0., 0.);
    //glm::vec3 size = glm::vec3(4930., 512., 51.);
    //glm::vec3 nb = glm::vec3(1., 1., 1.);
    //SimpleGrid grid("../../../../../../../data/datasets/tulane/v3/registration_subset/v3_a5_100_150_8bit_normalized_25.tif", nb);

    //glm::vec3 originalPosition = glm::vec3(1194., 20., 4.);

    //std::cout << "Point at 1194. 20. 4.: " << unsigned(grid.getValueFromPoint(originalPosition))  << " == 112" << std::endl;

    //origin = glm::vec3(2., 0., 0.);
    //size = glm::vec3(2465., 256., 25.);
    //nb = glm::vec3(1., 1., 1.);
    //SimpleGrid grid2("../../../../../../../data/datasets/tulane/v3/registration_subset/v3_a5_100_150_8bit_normalized_25.tif", nb);

    //glm::vec3 newPosition = (originalPosition/glm::vec3(2., 2., 2.))+origin;
    //std::cout << "Same point: " << unsigned(grid2.getValueFromPoint(newPosition)) << std::endl;
}

TEST_CASE("Deformable", "[grid]") {
    //glm::vec3 origin = glm::vec3(0., 0., 0.);
    //glm::vec3 size = glm::vec3(940, 510, 20);
    //glm::vec3 nb = glm::vec3(1., 1., 1.);

    //std::string filename = "../../../../Data/myTiff.tif";
    //SimpleGrid deformableGrid(filename, nb);
    //SimpleGrid initialGrid(filename, nb);

    //deformableGrid.movePoint(glm::vec3(1, 1, 1), glm::vec3(600, 0., 0.));
    //deformableGrid.writeDeformedGrid(initialGrid);
}

TEST_CASE("ReadSimpleImage", "[grid]") {
    //TIFFImage gridTiff("../../../../Data/v3_a5_100_150_8bit_normalized_25.tif");
    //std::cout << "Get simple value: " << unsigned(gridTiff.getValue(glm::vec3(1182, 8, 0))) << " == 181" << std::endl;

    //TIFFImage gridTiffFiji("../../../../Data/myTiff.tif");
    //std::cout << "Get simple value: " << unsigned(gridTiffFiji.getValue(glm::vec3(50, 29, 0))) << " == 162" << std::endl;
}
