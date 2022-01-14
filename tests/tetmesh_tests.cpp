#include <catch2/catch_test_macros.hpp>
#include "../grid/include/tetrahedral_mesh.hpp"
#include <chrono>

float epsilon = 0.0000001;

bool areEqual(glm::vec4 p1, glm::vec4 p2) {
    return glm::distance(p1, p2) < epsilon;
}

bool areEqual(glm::vec3 p1, glm::vec3 p2) {
    return glm::distance(p1, p2) < epsilon;
}

TEST_CASE("printTetNeighbors", "[tetmesh][.print]") {
    TetMesh tetMesh; 

    glm::vec3 origin = glm::vec3(0., 0., 0.);
    glm::vec3 size = glm::vec3(1., 1., 1.);
    glm::vec3 nb = glm::vec3(2., 2., 2.);
    tetMesh.buildGrid(nb, size, origin);

    for(int i = 0; i < tetMesh.mesh.size(); ++i) {
        for(int j = 0; j < 4; ++j) {
            std::cout << "On tet [" << i << "] neighbor [" << j << "] is [" << tetMesh.mesh[i].neighbors[j] << "]" << std::endl;
        }
    }
}

TEST_CASE("checkTetMeshBuildCube", "[tetmesh]") {
    TetMesh tetMesh; 

    glm::vec3 origin = glm::vec3(0., 0., 0.);
    glm::vec3 size = glm::vec3(1., 1., 1.);
    glm::vec3 nb = glm::vec3(4., 1., 2.);
    tetMesh.buildGrid(nb, size, origin);

    int idx1 = tetMesh.inTetraIdx(glm::vec3(0.01, 0.01, 0.5));
    CHECK(idx1 == 0);

    int idx2 = tetMesh.inTetraIdx(glm::vec3(0.5, 0.01, 0.01));
    CHECK(idx2 == 1);

    int idx3 = tetMesh.inTetraIdx(glm::vec3(0.01, 0.5, 0.01));
    CHECK(idx3 == 2);

    int idx4 = tetMesh.inTetraIdx(glm::vec3(0.9, 0.9, 0.9));
    CHECK(idx4 == 3);

    int idx5 = tetMesh.inTetraIdx(glm::vec3(0.5, 0.5, 0.5));
    CHECK(idx5 == 4);

    int idx6 = tetMesh.inTetraIdx(glm::vec3(1.01, 0.01, 0.5));
    CHECK(idx6 == 5);

    int idx7 = tetMesh.inTetraIdx(glm::vec3(1.5, 0.01, 0.01));
    CHECK(idx7 == 6);

    int idx8 = tetMesh.inTetraIdx(glm::vec3(1.01, 0.5, 0.01));
    CHECK(idx8 == 7);

    int idx9 = tetMesh.inTetraIdx(glm::vec3(1.9, 0.9, 0.9));
    CHECK(idx9 == 8);

    int idx10 = tetMesh.inTetraIdx(glm::vec3(1.5, 0.5, 0.5));
    CHECK(idx10 == 9);

    idx1 = tetMesh.inTetraIdx(glm::vec3(0.01, 0.01, 0.5));
    idx2 = tetMesh.inTetraIdx(glm::vec3(3.01, 0.01, 1.5));

    Tetrahedron tet1 = tetMesh.getTetra(idx1);
    Tetrahedron tet2 = tetMesh.getTetra(idx2);

    glm::vec4 coord1 = tet1.computeBaryCoord(glm::vec3(0.01, 0.01, 0.5));
    glm::vec4 coord2 = tet2.computeBaryCoord(glm::vec3(3.01, 0.01, 1.5));

    CHECK(areEqual(coord1, coord2));

    CHECK(areEqual(tet1.baryToWorldCoord(coord1), glm::vec3(0.01, 0.01, 0.5)));
}

TEST_CASE("checkTetMeshBaryCoord", "[tetmesh]") {

    TetMesh tetMesh; 
    glm::vec3 origin = glm::vec3(0., 0., 0.);
    glm::vec3 size = glm::vec3(1., 1., 1.);
    glm::vec3 nb = glm::vec3(2., 1., 1.);
    tetMesh.buildGrid(nb, size, origin);

    int idx1 = tetMesh.inTetraIdx(glm::vec3(0.01, 0.01, 0.5));
    int idx2 = tetMesh.inTetraIdx(glm::vec3(1.01, 0.01, 0.5));

    Tetrahedron tet1 = tetMesh.getTetra(idx1);
    Tetrahedron tet2 = tetMesh.getTetra(idx2);

    glm::vec4 coord1 = tet1.computeBaryCoord(glm::vec3(0.01, 0.01, 0.5));
    glm::vec4 coord2 = tet2.computeBaryCoord(glm::vec3(1.01, 0.01, 0.5));

    CHECK(areEqual(coord1, coord2));
}

TEST_CASE("checkTetMeshInTetFct", "[tetmesh]") {
    TetMesh tetMesh; 
    glm::vec3 origin = glm::vec3(0., 0., 0.);
    glm::vec3 size = glm::vec3(1., 1., 1.);
    glm::vec3 nb = glm::vec3(2., 1., 1.);
    tetMesh.buildGrid(nb, size, origin);

    int idx1 = tetMesh.inTetraIdx(glm::vec3(0.01, 0.01, 0.5));
    CHECK(idx1 == 0);

    int idx2 = tetMesh.inTetraIdx(glm::vec3(0.5, 0.01, 0.01));
    CHECK(idx2 == 1);

    int idx3 = tetMesh.inTetraIdx(glm::vec3(0.01, 0.5, 0.01));
    CHECK(idx3 == 2);

    int idx4 = tetMesh.inTetraIdx(glm::vec3(0.9, 0.9, 0.9));
    CHECK(idx4 == 3);

    int idx5 = tetMesh.inTetraIdx(glm::vec3(0.5, 0.5, 0.5));
    CHECK(idx5 == 4);

    int idx6 = tetMesh.inTetraIdx(glm::vec3(1.01, 0.01, 0.5));
    CHECK(idx6 == 5);

    int idx7 = tetMesh.inTetraIdx(glm::vec3(1.5, 0.01, 0.01));
    CHECK(idx7 == 6);

    int idx8 = tetMesh.inTetraIdx(glm::vec3(1.01, 0.5, 0.01));
    CHECK(idx8 == 7);

    int idx9 = tetMesh.inTetraIdx(glm::vec3(1.9, 0.9, 0.9));
    CHECK(idx9 == 8);

    int idx10 = tetMesh.inTetraIdx(glm::vec3(1.5, 0.5, 0.5));
    CHECK(idx10 == 9);
}

TEST_CASE("checkPerf", "[tetmesh]") {
    //TetMesh tetMesh;

    //glm::vec3 origin = glm::vec3(0., 0., 0.);
    //glm::vec3 size = glm::vec3(1., 1., 1.);
    //glm::vec3 nb = glm::vec3(100., 100., 100.);
    //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    //tetMesh.buildGrid(nb, size, origin);
    //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    //std::cout << "Building time: " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;

    //begin = std::chrono::steady_clock::now();
    //int idx1 = tetMesh.inTetraIdx(glm::vec3(0.01, 0.01, 0.5));
    //end = std::chrono::steady_clock::now();

    //std::cout << "Check in best time: " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;

    //begin = std::chrono::steady_clock::now();
    //idx1 = tetMesh.inTetraIdx(glm::vec3(99.01, 99.01, 99.5));
    //end = std::chrono::steady_clock::now();

    //std::cout << "Check in worst time: " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;

    //begin = std::chrono::steady_clock::now();
    //for(int i = 0; i < 100; ++i)
    //    idx1 = tetMesh.inTetraIdx(glm::vec3(((double)rand()/(RAND_MAX))*100.,  ((double)rand()/(RAND_MAX))*100., ((double)rand()/(RAND_MAX))*100.));
    //end = std::chrono::steady_clock::now();

    //std::cout << "Check in with random 100: " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;
}

