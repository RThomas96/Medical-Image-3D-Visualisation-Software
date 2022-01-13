#include <catch2/catch_test_macros.hpp>
#include "../grid/include/grid.hpp"

TEST_CASE("SaveDurationCacheComparison", "[grid][save][.long]") {

    glm::vec3 nb = glm::vec3(5., 5., 5.);
    SimpleGrid * initialGrid = new SimpleGrid("../../tests/data/img1.tif", nb, 4);
    SimpleGrid * deformedGrid = new SimpleGrid("../../tests/data/img1.tif", nb, 4);
    initialGrid->grid.image.useCache = true;
    deformedGrid->grid.image.useCache = true;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    deformedGrid->writeDeformedGrid(*initialGrid, ResolutionMode::SAMPLER_RESOLUTION);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    float durationWithCache3 = std::chrono::duration_cast<std::chrono::seconds> (end - begin).count();
    std::cout << "With cache of 3: "  << durationWithCache3 << "second" << std::endl;
    std::cout << "Number of insertion: " << initialGrid->grid.image.cache->nbInsertion << std::endl;

    /***/

    delete initialGrid;
    delete deformedGrid;
    initialGrid = new SimpleGrid("../../tests/data/img1.tif", nb, 4);
    deformedGrid = new SimpleGrid("../../tests/data/img1.tif", nb, 4);
    initialGrid->grid.image.useCache = true;
    deformedGrid->grid.image.useCache = true;
    initialGrid->grid.image.cache->setCapacity(10);

    begin = std::chrono::steady_clock::now();
    deformedGrid->writeDeformedGrid(*initialGrid, ResolutionMode::SAMPLER_RESOLUTION);
    end = std::chrono::steady_clock::now();

    float durationWithCache10 = std::chrono::duration_cast<std::chrono::seconds> (end - begin).count();
    std::cout << "With cache of 10: "  << durationWithCache10 << " second" << std::endl;
    std::cout << "Number of insertion: " << initialGrid->grid.image.cache->nbInsertion << std::endl;

    /***/

    delete initialGrid;
    delete deformedGrid;
    initialGrid = new SimpleGrid("../../tests/data/img1.tif", nb, 4);
    deformedGrid = new SimpleGrid("../../tests/data/img1.tif", nb, 4);
    initialGrid->grid.image.useCache = false;
    deformedGrid->grid.image.useCache = false;

    begin = std::chrono::steady_clock::now();
    deformedGrid->writeDeformedGrid(*initialGrid, ResolutionMode::SAMPLER_RESOLUTION);
    end = std::chrono::steady_clock::now();

    std::cout << "Without cache: "  << std::chrono::duration_cast<std::chrono::seconds> (end - begin).count() << " second" << std::endl;
    std::cout << "Number of insertion: " << initialGrid->grid.image.cache->nbInsertion << std::endl;
    float durationWithoutCache = std::chrono::duration_cast<std::chrono::seconds> (end - begin).count();

    /***/

    std::cout << "*****************" << std::endl;
    std::cout << "*****Results*****" << std::endl;
    std::cout << "*****************" << std::endl;
    std::cout << "With cache of 3: "  << durationWithCache3 << "second" << std::endl;
    std::cout << "With cache of 10: "  << durationWithCache10 << " second" << std::endl;
    std::cout << "Without cache: "  << durationWithoutCache << " second" << std::endl;
    std::cout << "*****************" << std::endl;
    std::cout << "Cache of 3 speed up the save process to: " << durationWithoutCache/durationWithCache3 << std::endl;
    std::cout << "Cache of 10 speed up the save process to: " << durationWithoutCache/durationWithCache10 << std::endl;
    std::cout << "For commit 3f998b2a79447c0c2f8e275999290a364923aac6 : values are 6, 6, 27, 4.5 and 4.5" << std::endl;
    std::cout << "For commit 3f998b2a79447c0c2f8e275999290a364923aac6 : values are 6, 6, 28, 4.667 and 4.667" << std::endl;
    std::cout << "*****************" << std::endl;
}

TEST_CASE("CutAndDivideResolution", "[grid]") {

    glm::vec3 nb = glm::vec3(5., 5., 5.);
    SimpleGrid grid("../../tests/data/img1.tif", nb, 4);

	std::vector<std::uint16_t> slices;

    int offsetOnZ = static_cast<int>(grid.grid.resolutionRatio[2]);
    int imgSizeZ = grid.grid.getImageDimensions()[2];

	//for (std::size_t s = 0; s < imgSizeZ; s+=offsetOnZ) {

    // Get first slice
    grid.grid.getGridSlice(0, slices, 1);
    CHECK(slices[309] == 9440);//x = 1236/4 = 309
    CHECK(slices[617+309] == 6574);// x size = 1233

    slices.clear();

    grid.grid.getGridSlice(0+offsetOnZ, slices, 1);
    CHECK(slices[309] == 5098);//x = 1236
    CHECK(slices[617+309] == 3965);
}

TEST_CASE("CutResolution", "[grid]") {

    glm::vec3 nb = glm::vec3(5., 5., 5.);
    SimpleGrid grid("../../tests/data/img1.tif", nb, 1);

	std::vector<std::uint16_t> slices;

    int offsetOnZ = static_cast<int>(grid.grid.resolutionRatio[2]);
    int imgSizeZ = grid.grid.getImageDimensions()[2];

	//for (std::size_t s = 0; s < imgSizeZ; s+=offsetOnZ) {

    // Get first slice
    grid.grid.getGridSlice(0, slices, 1);
    CHECK(slices[1236] == 9440);//x = 1236/4 = 309
    CHECK(slices[1236+(4930/2.)] == 7973);

    slices.clear();

    grid.grid.getGridSlice(0+offsetOnZ, slices, 1);
    CHECK(slices[1236] == 9005);//x = 1236
}

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
