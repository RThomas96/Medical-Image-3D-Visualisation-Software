#include "../include/tetrahedral_mesh.hpp"

float ScTP(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    return glm::dot(a, glm::cross(b, c));
}

bool SameSide(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& v4, const glm::vec3& p) {
    glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);
    float dotV4 = glm::dot(normal, v4 - v1);
    float dotP = glm::dot(normal, p - v1);
    return ((dotV4<0) == (dotP<0));
}

std::vector<glm::vec3*> insertCube(std::vector<glm::vec3> cubePts, glm::vec3 indices, std::vector<std::vector<std::vector<glm::vec3>>>& ptGrid) {
    int x = indices[0];
    int y = indices[1];
    int z = indices[2];
    ptGrid[x][y][z] = cubePts[0];
    ptGrid[x][y+1][z] = cubePts[1];
    ptGrid[x+1][y+1][z] = cubePts[2];
    ptGrid[x+1][y][z] = cubePts[3];


    ptGrid[x][y][z+1] = cubePts[4];
    ptGrid[x][y+1][z+1] = cubePts[5];
    ptGrid[x+1][y+1][z+1] = cubePts[6];
    ptGrid[x+1][y][z+1] = cubePts[7];

    std::vector<glm::vec3*> res;
    res.push_back(&ptGrid[x][y][z]);
    res.push_back(&ptGrid[x][y+1][z]);
    res.push_back(&ptGrid[x+1][y+1][z]);
    res.push_back(&ptGrid[x+1][y][z]);

    res.push_back(&ptGrid[x][y][z+1]);
    res.push_back(&ptGrid[x][y+1][z+1]);
    res.push_back(&ptGrid[x+1][y+1][z+1]);
    res.push_back(&ptGrid[x+1][y][z+1]);
    
    return res;
}

std::vector<glm::vec3> buildCube(const glm::vec3& origin, const glm::vec3& size) {
    std::vector<glm::vec3> cube;
    cube.push_back(origin);
    cube.push_back(origin + glm::vec3(0., size[1], 0.));
    cube.push_back(origin + glm::vec3(size[0], size[1], 0.));
    cube.push_back(origin + glm::vec3(size[0], 0., 0.));

    cube.push_back(origin + glm::vec3(0., 0., size[2]));
    cube.push_back(origin + glm::vec3(0., size[1], size[2]));
    cube.push_back(origin + glm::vec3(size[0], size[1], size[2]));
    cube.push_back(origin + glm::vec3(size[0], 0., size[2]));
    return cube;
}

glm::vec3 idxTo3D(int i, int h, int w) {
    return glm::vec3(i%h, static_cast<int>(std::floor(i/h))%w, std::floor(i/(h*w)));
}


Tetrahedron::Tetrahedron() {
    this->points[0] = nullptr;
    this->points[1] = nullptr;
    this->points[2] = nullptr;
    this->points[3] = nullptr;
}

Tetrahedron::Tetrahedron(glm::vec3* a, glm::vec3* b, glm::vec3* c, glm::vec3* d) {
    this->points[0] = a; 
    this->points[1] = b;
    this->points[2] = c;
    this->points[3] = d;
}

glm::vec4 Tetrahedron::computeBaryCoord(const glm::vec3& p) {
    //glm::vec4 bary_tet(const glm::vec3 & a, const glm::vec3 & b, const glm::vec3 & c, const glm::vec3 & d, const glm::vec3 & p)
    const glm::vec3& a = *this->points[0];
    const glm::vec3& b = *this->points[1];
    const glm::vec3& c = *this->points[2];
    const glm::vec3& d = *this->points[3];

    glm::vec3 vap = p - a;
    glm::vec3 vbp = p - b;

    glm::vec3 vab = b - a;
    glm::vec3 vac = c - a;
    glm::vec3 vad = d - a;

    glm::vec3 vbc = c - b;
    glm::vec3 vbd = d - b;
    // ScTP computes the scalar triple product
    float va6 = ScTP(vbp, vbd, vbc);
    float vb6 = ScTP(vap, vac, vad);
    float vc6 = ScTP(vap, vad, vab);
    float vd6 = ScTP(vap, vab, vac);
    float v6 = 1. / ScTP(vab, vac, vad);
    return glm::vec4(va6*v6, vb6*v6, vc6*v6, vd6*v6);
}

bool Tetrahedron::isInTetrahedron(const glm::vec3& p) {
    const glm::vec3& v1 = *this->points[0];
    const glm::vec3& v2 = *this->points[1];
    const glm::vec3& v3 = *this->points[2];
    const glm::vec3& v4 = *this->points[3];
    return SameSide(v1, v2, v3, v4, p) && SameSide(v2, v3, v4, v1, p) && SameSide(v3, v4, v1, v2, p) && SameSide(v4, v1, v2, v3, p);
}

glm::vec3 Tetrahedron::baryToWorldCoord(const glm::vec4& coord) {
    const glm::vec3& v1 = *this->points[0];
    const glm::vec3& v2 = *this->points[1];
    const glm::vec3& v3 = *this->points[2];
    const glm::vec3& v4 = *this->points[3];
    float x = coord[0]*v1[0] + coord[1]*v2[0] + coord[2]*v3[0] + coord[3]*v4[0];
    float y = coord[0]*v1[1] + coord[1]*v2[1] + coord[2]*v3[1] + coord[3]*v4[1];
    float z = coord[0]*v1[2] + coord[1]*v2[2] + coord[2]*v3[2] + coord[3]*v4[2];
    return glm::vec3(x, y, z);
}

void TetMesh::buildGrid(const glm::vec3& nbCube, const glm::vec3& sizeCube, const glm::vec3& origin) {
    if(!this->isEmpty())
        throw std::runtime_error("Error: build grid cannot be used on already constructed mesh.");
    // Reason: nbTetra, bbMin and bbMax aren't correctly updated when build grid is used on already constructed mesh
    this->nbTetra += nbCube;
    this->bbMin = origin;
    this->bbMax = origin+nbCube * sizeCube;

    // We add +1 here because we stock points, and there one more point than cube as it need a final point
    ptGrid = std::vector<std::vector<std::vector<glm::vec3>>>(nbCube[0]+1, std::vector<std::vector<glm::vec3>>(nbCube[1]+1, std::vector<glm::vec3>(nbCube[2]+1, glm::vec3(0., 0., 0.))));

    for(int k = 0; k < nbCube[2]; ++k) {
        for(int j = 0; j < nbCube[1]; ++j) {
            for(int i = 0; i < nbCube[0]; ++i) {
                glm::vec3 offset = glm::vec3(i*sizeCube[0], j*sizeCube[1], k*sizeCube[2]);
                std::vector<glm::vec3> cubePts = buildCube(origin+offset, sizeCube);
                std::vector<glm::vec3*> cubePtsAdress = insertCube(cubePts, glm::vec3(i, j, k), this->ptGrid);
                this->addCube(cubePtsAdress);
            }
        }
    }
}

void TetMesh::movePoint(const glm::vec3& indices, const glm::vec3& position) {
    glm::vec3& p = this->ptGrid[indices[0]][indices[1]][indices[2]];
    std::cout << "Move point: " << p << " -> ";
    p += position;
    std::cout << p << std::endl;
    // Update bbs
    for(int i = 0; i < 3; ++i) {
        if(p[i] > this->bbMax[i])
            this->bbMax[i] = p[i];

        if(p[i] < this->bbMin[i])
            this->bbMin[i] = p[i];
    }
}

bool TetMesh::isEmpty() const {
    return this->mesh.empty();
}

Tetrahedron TetMesh::getTetra(int idx) const {
    return mesh[idx];
} 

int TetMesh::inTetraIdx(const glm::vec3& p) {
    int i = 0;
    for(int i = 0; i < mesh.size(); ++i)
        if(mesh[i].isInTetrahedron(p))
            return i;
    return -1;
}

glm::vec3 TetMesh::getDimensions() {
    return this->bbMax - this->bbMin;
}

// Temporary function to map to current GL
void TetMesh::replaceAllPoints(const std::vector<glm::vec3>& pts) {
    int l = 0;
    if((nbTetra[0]+1) * (nbTetra[1]+1) * (nbTetra[2]+1) != pts.size())
        std::cout << "Wrong pts number !!" << std::endl;
    for(int k = 0; k < this->nbTetra[2] + 1; ++k) {
        for(int j = 0; j < this->nbTetra[1] + 1; ++j) {
            for(int i = 0; i < this->nbTetra[0] + 1; ++i) {
                this->ptGrid[i][j][k] = pts[l];
                const glm::vec3& p = pts[l];
                for(int m = 0; m < 3; ++m) {
                    if(p[m] > this->bbMax[m])
                        this->bbMax[m] = p[m];

                    if(p[m] < this->bbMin[m])
                        this->bbMin[m] = p[m];
                }
                ++l;
            }
        }
    }
}

// This function is private because it doesn't update fields nbTetra, bbMin and bbMax
// Thus it can only be used in buildGrid function
void TetMesh::addCube(std::vector<glm::vec3*> pts) {
    if(pts.size() > 8) {
        std::cerr << "Error: can't add a cube with more than 8 vertices" << std::endl;
        return;
    }
    mesh.push_back(Tetrahedron(pts[0], pts[5], pts[7], pts[4]));
    mesh.push_back(Tetrahedron(pts[0], pts[7], pts[2], pts[3]));
    mesh.push_back(Tetrahedron(pts[0], pts[2], pts[5], pts[1]));
    mesh.push_back(Tetrahedron(pts[2], pts[7], pts[5], pts[6]));
    mesh.push_back(Tetrahedron(pts[0], pts[7], pts[5], pts[2]));
}

void check1DTo3D() {
    int h = 3;
    int w = 5;
    for(int i = 0; i < 60; ++i) {
        std::cout << i << " : " << idxTo3D(i, h, w) << std::endl;
    }
}

/* Unit test */
