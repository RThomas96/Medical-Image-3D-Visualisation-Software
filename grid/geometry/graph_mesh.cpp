#include "graph_mesh.hpp"
#include <iostream>
#include <fstream>

void GraphMesh::loadMESH(std::string const &filename) {
    std::cout << "Opening " << filename << std::endl;

    std::ifstream myfile;
    myfile.open(filename.c_str());
    if (! myfile.is_open()) {
        std::cout << filename << " cannot be opened" << std::endl;
        return;
    }

    std::string magic_s;
    myfile >> magic_s;
    if (magic_s != "OFF") {
        std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
        myfile.close();
        exit(1);
    }

    int n_vertices, n_faces, n_edges;
    myfile >> n_vertices >> n_faces >> n_edges;

    if (n_edges == 0) {
        std::cout << n_edges << " = 0 : Graph file empty !." << std::endl;
        myfile.close();
        exit(1);
    }

    this->vertices.clear();
    for (int v = 0; v < n_vertices; ++v)
    {
        float x, y, z;
        myfile >> x >> y >> z;
        this->vertices.push_back(glm::vec3(x, y, z));
    }

    for (int v = 0; v < n_edges; ++v)
    {
        int dummy, a, b;
        myfile >> dummy >> a >> b;
        this->mesh.push_back(GraphEdge(a-1, b-1));
    }
}
