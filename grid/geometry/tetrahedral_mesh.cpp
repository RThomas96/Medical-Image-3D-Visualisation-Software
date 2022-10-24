#include "tetrahedral_mesh.hpp"
//#include "../deformation/mesh_deformer.hpp"
#include <map>
#include <algorithm>
#include <fstream>
#include <random>


// This function make the link between a face of a tetrahedron and its points
// This ensure a stable way to iterate throught faces of a tetrahedron
// These are global variable as it is common to all tetrahedron
// You can also see the faceIdx as the point index which in the opposite side of the face
//std::size_t faceOrder[4][3] = {{3, 1, 2}, {3, 2, 0}, {3, 0, 1}, {2, 1, 0}};
std::size_t faceOrder[4][3] = {{3, 1, 2}, {3, 2, 0}, {3, 0, 1}, {2, 1, 0}};
int getIdxOfPtInFace(int faceIdx, int pointIdx) {
    return faceOrder[faceIdx][pointIdx];
}

int TetMesh::from3DTo1D(const glm::vec3& p) const {
    return p[0] + (this->nbTetra[0] + 1)*p[1] + (this->nbTetra[0] + 1)*(this->nbTetra[1] + 1)*p[2];
}

float ScTP(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    return glm::dot(a, glm::cross(b, c));
}

bool SameSide(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& v4, const glm::vec3& p) {
    glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);
    float dotV4 = glm::dot(normal, v4 - v1);
    float dotP = glm::dot(normal, p - v1);
    return ((dotV4<0) == (dotP<0));
}

int Tetrahedron::getPointIndex(int faceIdx, int ptIdxInFace) const{
    return this->pointsIdx[getIdxOfPtInFace(faceIdx, ptIdxInFace)];
}

glm::vec3 * Tetrahedron::getPoint(int faceIdx, int ptIdxInFace) const {
    return this->points[getIdxOfPtInFace(faceIdx, ptIdxInFace)];
}

TetMesh::TetMesh(): nbTetra(glm::vec3(0., 0., 0.)), mesh(std::vector<Tetrahedron>()) {}

TetMesh::~TetMesh(){}

std::vector<glm::vec3*> TetMesh::insertCubeIntoPtGrid(std::vector<glm::vec3> cubePts, glm::vec3 indices, std::vector<glm::vec3>& vertices, std::vector<int>& ptIndices) {
    int x = indices[0];
    int y = indices[1];
    int z = indices[2];

    ptIndices[0] = this->from3DTo1D(glm::vec3(x,y,z));
    ptIndices[1] = this->from3DTo1D(glm::vec3(x,y+1,z));
    ptIndices[2] = this->from3DTo1D(glm::vec3(x+1,y+1,z));
    ptIndices[3] = this->from3DTo1D(glm::vec3(x+1,y,z));

    ptIndices[4] = this->from3DTo1D(glm::vec3(x,y,z+1));
    ptIndices[5] = this->from3DTo1D(glm::vec3(x,y+1,z+1));
    ptIndices[6] = this->from3DTo1D(glm::vec3(x+1,y+1,z+1));
    ptIndices[7] = this->from3DTo1D(glm::vec3(x+1,y,z+1));

    vertices[ptIndices[0]] = cubePts[0];
    vertices[ptIndices[1]] = cubePts[1];
    vertices[ptIndices[2]] = cubePts[2];
    vertices[ptIndices[3]] = cubePts[3];

    vertices[ptIndices[4]] = cubePts[4];
    vertices[ptIndices[5]] = cubePts[5];
    vertices[ptIndices[6]] = cubePts[6];
    vertices[ptIndices[7]] = cubePts[7];

    glm::vec3 tetMeshSize = bbMax - bbMin;
    texCoord[ptIndices[0]] = cubePts[0]/tetMeshSize;
    texCoord[ptIndices[1]] = cubePts[1]/tetMeshSize;
    texCoord[ptIndices[2]] = cubePts[2]/tetMeshSize;
    texCoord[ptIndices[3]] = cubePts[3]/tetMeshSize;

    texCoord[ptIndices[4]] = cubePts[4]/tetMeshSize;
    texCoord[ptIndices[5]] = cubePts[5]/tetMeshSize;
    texCoord[ptIndices[6]] = cubePts[6]/tetMeshSize;
    texCoord[ptIndices[7]] = cubePts[7]/tetMeshSize;

    std::vector<glm::vec3*> res;
    res.push_back(&vertices[ptIndices[0]]);
    res.push_back(&vertices[ptIndices[1]]);
    res.push_back(&vertices[ptIndices[2]]);
    res.push_back(&vertices[ptIndices[3]]);

    res.push_back(&vertices[ptIndices[4]]);
    res.push_back(&vertices[ptIndices[5]]);
    res.push_back(&vertices[ptIndices[6]]);
    res.push_back(&vertices[ptIndices[7]]);
    
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

void Tetrahedron::setIndices(int a, int b, int c, int d) {
    this->pointsIdx[0] = a;
    this->pointsIdx[1] = b;
    this->pointsIdx[2] = c;
    this->pointsIdx[3] = d;
}

Tetrahedron::Tetrahedron() {
    this->points[0] = nullptr;
    this->points[1] = nullptr;
    this->points[2] = nullptr;
    this->points[3] = nullptr;

    this->pointsIdx[0] = -1;
    this->pointsIdx[1] = -1;
    this->pointsIdx[2] = -1;
    this->pointsIdx[3] = -1;

    this->neighbors[0] = -1;
    this->neighbors[1] = -1;
    this->neighbors[2] = -1;
    this->neighbors[3] = -1;

    this->normals[0] = glm::vec4(0., 0., 0., 0.);
    this->normals[1] = glm::vec4(0., 0., 0., 0.);
    this->normals[2] = glm::vec4(0., 0., 0., 0.);
    this->normals[3] = glm::vec4(0., 0., 0., 0.);
}

Tetrahedron::Tetrahedron(glm::vec3* a, glm::vec3* b, glm::vec3* c, glm::vec3* d) {
    this->points[0] = a; 
    this->points[1] = b;
    this->points[2] = c;
    this->points[3] = d;

    this->pointsIdx[0] = -1;
    this->pointsIdx[1] = -1;
    this->pointsIdx[2] = -1;
    this->pointsIdx[3] = -1;

    this->neighbors[0] = -1;
    this->neighbors[1] = -1;
    this->neighbors[2] = -1;
    this->neighbors[3] = -1;

    this->normals[0] = glm::vec4(0., 0., 0., 0.);
    this->normals[1] = glm::vec4(0., 0., 0., 0.);
    this->normals[2] = glm::vec4(0., 0., 0., 0.);
    this->normals[3] = glm::vec4(0., 0., 0., 0.);
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

//bool Tetrahedron::isInTetrahedron(const glm::vec3& p) const {
//    const glm::vec3& v1 = *this->points[0];
//    const glm::vec3& v2 = *this->points[1];
//    const glm::vec3& v3 = *this->points[2];
//    const glm::vec3& v4 = *this->points[3];
//    return SameSide(v1, v2, v3, v4, p) && SameSide(v2, v3, v4, v1, p) && SameSide(v3, v4, v1, v2, p) && SameSide(v4, v1, v2, v3, p);
//}

float det(const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
    return b[0]*c[1]*d[2] + c[0]*d[1]*b[2] + d[0]*b[1]*c[2] - d[0]*c[1]*b[2] - c[0]*b[1]*d[2] - b[0]*d[1]*c[2];
}

bool Tetrahedron::isInTetrahedron(const glm::vec3& p) const {
    const glm::vec3& v0 = *this->points[0];
    const glm::vec3& v1 = *this->points[1];
    const glm::vec3& v2 = *this->points[2];
    const glm::vec3& v3 = *this->points[3];

    const glm::vec3& a = v0 - p;
    const glm::vec3& b = v1 - p;
    const glm::vec3& c = v2 - p;
    const glm::vec3& d = v3 - p;
    const float detA = det(b,c,d);
    const float detB = det(a,c,d);
    const float detC = det(a,b,d);
    const float detD = det(a,b,c);
    bool ret0 = detA > 0.0 && detB < 0.0 && detC > 0.0 && detD < 0.0;
    bool ret1 = detA < 0.0 && detB > 0.0 && detC < 0.0 && detD > 0.0;
    return ret0 || ret1;
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

glm::vec3 Tetrahedron::baryToCoord(const glm::vec4& coord, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& v4) {
    float x = coord[0]*v1[0] + coord[1]*v2[0] + coord[2]*v3[0] + coord[3]*v4[0];
    float y = coord[0]*v1[1] + coord[1]*v2[1] + coord[2]*v3[1] + coord[3]*v4[1];
    float z = coord[0]*v1[2] + coord[1]*v2[2] + coord[2]*v3[2] + coord[3]*v4[2];
    return glm::vec3(x, y, z);
}

void Tetrahedron::computeNormals() {
    for(int faceIdx = 0; faceIdx < 4; ++faceIdx) {
        glm::vec3 p0 = glm::vec3(glm::vec4(*this->points[getIdxOfPtInFace(faceIdx, 0)], 1.));
        glm::vec3 p1 = glm::vec3(glm::vec4(*this->points[getIdxOfPtInFace(faceIdx, 1)], 1.));
        glm::vec3 p2 = glm::vec3(glm::vec4(*this->points[getIdxOfPtInFace(faceIdx, 2)], 1.));

        glm::vec3 pF  = glm::vec3(glm::vec4(*this->points[faceIdx], 1.));
        glm::vec3 pFO = glm::vec3(glm::vec4(*this->points[(faceIdx + 1) % 4], 1.));

        glm::vec3 n1   = p1 - p0;
        glm::vec3 n2   = p2 - p0;
        glm::vec4 norm = glm::normalize(glm::vec4(glm::cross(glm::vec3(n1), glm::vec3(n2)), 1.));
        // Put inverse of dot with opposing vertex in norm.w :
        glm::vec4 v1			  = glm::vec4(pF - pFO, 1.);
        glm::vec4::value_type val = 1. / glm::dot(v1, norm);
        norm.w					  = val;
        this->normals[faceIdx] = norm;
    }
}

void Tetrahedron::getCentroid(glm::vec3& centroid) const {
    centroid = glm::vec3(0., 0., 0.);
    for(int i = 0; i < 4; ++i) {
        centroid += *this->points[i];
    }
    centroid /= 4.f;
}

glm::vec3 Tetrahedron::getBBMin() const {
    glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    for(int i = 0; i < 4; ++i) {
        if(min.x > this->points[i]->x)
            min.x = this->points[i]->x;

        if(min.y > this->points[i]->y)
            min.y = this->points[i]->y;

        if(min.z > this->points[i]->z)
            min.z = this->points[i]->z;
    }
    return min;
}

glm::vec3 Tetrahedron::getBBMax() const {
    glm::vec3 max = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);
    for(int i = 0; i < 4; ++i) {
        if(max.x < this->points[i]->x)
            max.x = this->points[i]->x;

        if(max.y < this->points[i]->y)
            max.y = this->points[i]->y;

        if(max.z < this->points[i]->z)
            max.z = this->points[i]->z;
    }
    return max;
}

//bool Tetrahedron::faceIntersect(const glm::vec3& p1, const glm::vec3& p2, int faceIdx) const {
//    // Möller–Trumbore intersection algorithm
//    glm::vec3 rayOrigin = p1;
//    glm::vec3 rayDir = (p2 - p1);
//
//    glm::vec3 triEdge1 = (*this->getPoint(faceIdx, 1) - *this->getPoint(faceIdx, 0));
//    glm::vec3 triEdge2 = (*this->getPoint(faceIdx, 2) - *this->getPoint(faceIdx, 0));
//
//    glm::vec3 h = glm::cross(rayDir, triEdge2);
//    float dot = glm::dot(triEdge1, h);
//    if (std::abs(dot) <  1.e-6)
//        return false; // Ray parallel to the triangle
//    float f = 1.f/dot;
//    glm::vec3 s = (rayOrigin - *this->getPoint(faceIdx, 0));
//    float u = f * glm::dot(s, h);
//    if (u < 0.f || 1.f < u)
//        return false; // Ray did not reach the triangle
//    glm::vec3 q = glm::cross(s, triEdge1);
//    float v = f * glm::dot(rayDir, q);
//    if (v < 0.f || 1.f < (u + v))
//        return false;
//
//    return true;
//}
//

bool Tetrahedron::planeIntersect(const glm::vec3& origin, const glm::vec3& direction) const {
    return !((glm::dot(*this->points[0] - origin, direction) > 0.) ==
             (glm::dot(*this->points[1] - origin, direction) > 0.) ==
             (glm::dot(*this->points[2] - origin, direction) > 0.) ==
             (glm::dot(*this->points[3] - origin, direction) > 0.));
}

bool Tetrahedron::faceIntersect(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) const {
    const glm::vec3& orig = p1;
    const glm::vec3& dir = glm::normalize(p2 - p1);
    //const glm::vec3& v0 = *this->getPoint(faceIdx, 0);
    //const glm::vec3& v1 = *this->getPoint(faceIdx, 1);
    //const glm::vec3& v2 = *this->getPoint(faceIdx, 2);
    float t, u, v;

    const glm::vec3 v0v1 = v1 - v0;
    const glm::vec3 v0v2 = v2 - v0;
    const glm::vec3 pvec = glm::cross(dir, v0v2);
    float det = glm::dot(v0v1, pvec);
    float invDet = 1.0f / det;

    if (det < 0.000001) return false;

    glm::vec3 tvec = orig - v0;
    u = glm::dot(tvec, pvec) * invDet;
    if (u < 0.f || u > 1.f) return false;

    glm::vec3 qvec = glm::cross(tvec, v0v1);
    v = glm::dot(dir, qvec) * invDet;
    if (v < 0.f || u + v > 1.f) return false;

    //t = glm::dot(v0v2, qvec) * invDet;

    return true;
}

//bool Tetrahedron::faceIntersect(const glm::vec3& p1, const glm::vec3& p2, int faceIdx) const {
//    //bool intersectPlane(const Vec3f &n, const Vec3f &p0, const Vec3f &l0, const Vec3f &l, float &t)
//
//    const glm::vec3 e1 = (*this->getPoint(faceIdx, 1) - *this->getPoint(faceIdx, 0));
//    const glm::vec3 e2 = (*this->getPoint(faceIdx, 2) - *this->getPoint(faceIdx, 0));
//    const glm::vec3 n = glm::normalize(glm::cross(e1, e2));
//    const glm::vec3 p0 = (*this->getPoint(faceIdx, 2) + *this->getPoint(faceIdx, 1) + *this->getPoint(faceIdx, 0))/3.f;
//    const glm::vec3 l0 = p1;
//    const glm::vec3 l = glm::normalize(p2 - p1);
//
//    // assuming vectors are all normalized
//    float denom = glm::dot(n, l);
//    if (denom > 1e-6) {
//        glm::vec3 p0l0 = p0 - l0;
//        float t = glm::dot(p0l0, n) / denom;
//        return (t >= 0);
//    }
//
//    return false;
//}


void TetMesh::buildGrid(const glm::vec3& nbCube, const glm::vec3& sizeCube, const glm::vec3& origin) {
    if(!this->isEmpty())
        throw std::runtime_error("Error: build grid cannot be used on already constructed mesh.");
    // Reason: nbTetra, bbMin and bbMax aren't correctly updated when build grid is used on already constructed mesh
    this->nbTetra += nbCube;
    this->bbMin = origin;
    this->bbMax = origin+nbCube * sizeCube;

    // We add +1 here because we stock points, and there one more point than cube as it need a final point
    vertices = std::vector<glm::vec3>((this->nbTetra[0]+1)*(this->nbTetra[1]+1)*(this->nbTetra[2]+1), glm::vec3(0., 0., 0.));
    texCoord = std::vector<glm::vec3>((this->nbTetra[0]+1)*(this->nbTetra[1]+1)*(this->nbTetra[2]+1), glm::vec3(0., 0., 0.));

    std::vector<int> ptIndices(8, -1);
    for(int k = 0; k < nbCube[2]; ++k) {
        for(int j = 0; j < nbCube[1]; ++j) {
            for(int i = 0; i < nbCube[0]; ++i) {
                glm::vec3 offset = glm::vec3(i*sizeCube[0], j*sizeCube[1], k*sizeCube[2]);
                std::vector<glm::vec3> cubePts = buildCube(origin+offset, sizeCube);
                std::vector<glm::vec3*> cubePtsAdress = insertCubeIntoPtGrid(cubePts, glm::vec3(i, j, k), this->vertices, ptIndices);
                this->decomposeAndAddCube(cubePtsAdress, ptIndices);
            }
        }
    }
    this->computeNeighborhood();
    this->computeNormals();
}

bool TetMesh::isEmpty() const {
    return this->mesh.empty();
}

Tetrahedron TetMesh::getTetra(int idx) const {
    return mesh[idx];
} 

int TetMesh::inTetraIdx(const glm::vec3& p) const {
    // Naive version
    int i = 0;
    for(int i = 0; i < mesh.size(); ++i)
        if(mesh[i].isInTetrahedron(p))
            return i;
    return -1;
    //auto rng = std::default_random_engine {};

    //std::uniform_int_distribution<std::mt19937::result_type> dist(0,this->mesh.size()-1);
    //std::random_device dev;
    //std::mt19937 rng2(dev());
    //int seedIdx = dist(rng2);

    //int idx = seedIdx;
    //glm::vec3 centroid(0., 0., 0.);
    //do {
    //    std::vector<int> validNeigh;
    //    validNeigh.clear();
    //    const Tetrahedron& tet = this->mesh[idx];
    //    tet.getCentroid(centroid);

    //    for(int i = 0; i < 4; ++i) {
    //        const glm::vec3& p1 = this->vertices[tet.getPointIndex(i, 0)];
    //        const glm::vec3& p2 = this->vertices[tet.getPointIndex(i, 1)];
    //        const glm::vec3& p3 = this->vertices[tet.getPointIndex(i, 2)];

    //        if(tet.faceIntersect(p, centroid, p1, p2, p3)) {
    //            if(tet.neighbors[i] != -1) {
    //                validNeigh.push_back(tet.neighbors[i]);
    //            }
    //        }
    //    }
    //    if(!validNeigh.empty()) {
    //        if(tet.isInTetrahedron(p)) {
    //            //std::cout << "Found" << std::endl;
    //            return idx;
    //        }
    //        //if(validNeigh.size() > 1)
    //            //std::cout << "BUG" << std::endl;
    //        // This slow the computation a lot, but this is mandatory to avoid infinite loop in degenerate cases
    //        // https://hal.inria.fr/inria-00072509/document : at the end of page 11, fig.5
    //        //std::shuffle(std::begin(validNeigh), std::end(validNeigh), rng);
    //        idx = validNeigh[0];
    //        //std::cout << idx << std::endl;
    //    } else {
    //        //std::cout << "Naïve" << std::endl;
    //        idx = -1;
    //        int i = 0;
    //        for(int i = 0; i < mesh.size(); ++i)
    //            if(mesh[i].isInTetrahedron(p))
    //                return i;
    //        return -1;
    //    }
    //} while(true);
    //return idx;
}

// This function is private because it doesn't update fields nbTetra, bbMin and bbMax
// Thus it can only be used in buildGrid function
void TetMesh::decomposeAndAddCube(std::vector<glm::vec3*> pts, const std::vector<int>& ptsIdx) {
    if(pts.size() > 8) {
        std::cerr << "Error: can't add a cube with more than 8 vertices" << std::endl;
        return;
    }
    mesh.push_back(Tetrahedron(pts[3], pts[2], pts[1], pts[6]));
    mesh.back().setIndices(ptsIdx[3], ptsIdx[2], ptsIdx[1], ptsIdx[6]);

    mesh.push_back(Tetrahedron(pts[4], pts[0], pts[5], pts[7]));
    mesh.back().setIndices(ptsIdx[4], ptsIdx[0], ptsIdx[5], ptsIdx[7]);

    mesh.push_back(Tetrahedron(pts[5], pts[3], pts[6], pts[7]));
    mesh.back().setIndices(ptsIdx[5], ptsIdx[3], ptsIdx[6], ptsIdx[7]);

    mesh.push_back(Tetrahedron(pts[0], pts[3], pts[5], pts[7]));
    mesh.back().setIndices(ptsIdx[0], ptsIdx[3], ptsIdx[5], ptsIdx[7]);

    mesh.push_back(Tetrahedron(pts[0], pts[3], pts[1], pts[5]));
    mesh.back().setIndices(ptsIdx[0], ptsIdx[3], ptsIdx[1], ptsIdx[5]);

    mesh.push_back(Tetrahedron(pts[1], pts[3], pts[6], pts[5]));
    mesh.back().setIndices(ptsIdx[1], ptsIdx[3], ptsIdx[6], ptsIdx[5]);

    //3 . 2 . 1 . 6
    //4 . 0 . 5 . 7
    //5 . 3 . 6 . 7
    //0 . 3 . 5 . 7
    //0 . 3 . 1 . 5
    //1 . 3 . 6 . 5
}

void check1DTo3D() {
    int h = 3;
    int w = 5;
    for(int i = 0; i < 60; ++i) {
        std::cout << i << " : " << idxTo3D(i, h, w) << std::endl;
    }
}

/// @brief Helper struct to store the indices of vertices that make a face, and compare two faces to one another.
struct Face
{
public:
	inline Face(unsigned int v0, unsigned int v1, unsigned int v2) {
		if (v1 < v0)
			std::swap(v0, v1);
		if (v2 < v1)
			std::swap(v1, v2);
		if (v1 < v0)
			std::swap(v0, v1);
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
	}
	inline Face(const Face& f) {
		v[0] = f.v[0];
		v[1] = f.v[1];
		v[2] = f.v[2];
	}
	inline virtual ~Face() {}
	inline Face& operator=(const Face& f) {
		v[0] = f.v[0];
		v[1] = f.v[1];
		v[2] = f.v[2];
		return (*this);
	}
	inline bool operator==(const Face& f) { return (v[0] == f.v[0] && v[1] == f.v[1] && v[2] == f.v[2]); }
	inline bool operator<(const Face& f) const { return (v[0] < f.v[0] || (v[0] == f.v[0] && v[1] < f.v[1]) || (v[0] == f.v[0] && v[1] == f.v[1] && v[2] < f.v[2])); }
	inline bool contains(unsigned int i) const { return (v[0] == i || v[1] == i || v[2] == i); }
	inline unsigned int getVertex(unsigned int i) const { return v[i]; }
	unsigned int v[3];
};

void TetMesh::computeNeighborhood() {
	// generate the correspondance by looking at which faces are similar
	std::map<Face, std::pair<int, int>> adjacent_faces;
	for (std::size_t tetIdx = 0; tetIdx < this->mesh.size(); tetIdx++) {
        Tetrahedron& tet = this->mesh[tetIdx];
		for (int faceIdx = 0; faceIdx < 4; faceIdx++) {
			Face face = Face(tet.pointsIdx[getIdxOfPtInFace(faceIdx,0)],
                             tet.pointsIdx[getIdxOfPtInFace(faceIdx,1)],
                             tet.pointsIdx[getIdxOfPtInFace(faceIdx,2)]);
			std::map<Face, std::pair<int, int>>::iterator it = adjacent_faces.find(face);
			if (it == adjacent_faces.end()) {
				adjacent_faces[face] = std::make_pair(static_cast<int>(tetIdx), faceIdx);
			} else {
				tet.neighbors[faceIdx]						              = it->second.first;
				this->mesh[it->second.first].neighbors[it->second.second] = tetIdx;
			}
		}
	}
}

void TetMesh::computeNormals() {
    for(int tetIdx = 0; tetIdx < this->mesh.size(); ++tetIdx) {
        this->mesh[tetIdx].computeNormals();
    }
}

bool TetMesh::getCoordInInitial(const TetMesh& initial, const glm::vec3& p, glm::vec3& out, int tetraIdx) const {
    if(tetraIdx == -1) {
        tetraIdx = this->inTetraIdx(p);
    }
    if(tetraIdx != -1) {
        glm::vec4 baryCoordInDeformed = this->getTetra(tetraIdx).computeBaryCoord(p);
        glm::vec3 coordInInitial = initial.getTetra(tetraIdx).baryToWorldCoord(baryCoordInDeformed);
        out = coordInInitial;
        return true;
    } else {
        return false;
    }
}

bool TetMesh::getCoordInInitialOut(const TetMesh& initial, const glm::vec3& p, glm::vec3& out, int& tetraIdx) const {
    tetraIdx = this->inTetraIdx(p);
    if(tetraIdx != -1) {
        glm::vec4 baryCoordInDeformed = this->getTetra(tetraIdx).computeBaryCoord(p);
        glm::vec3 coordInInitial = initial.getTetra(tetraIdx).baryToWorldCoord(baryCoordInDeformed);
        out = coordInInitial;
        return true;
    } else {
        return false;
    }
}

bool TetMesh::getCoordInImage(const glm::vec3& p, glm::vec3& out, int tetraIdx) const {
    if(tetraIdx == -1) {
        tetraIdx = this->inTetraIdx(p);
    }
    if(tetraIdx != -1) {
        glm::vec4 baryCoordInDeformed = this->getTetra(tetraIdx).computeBaryCoord(p);
        glm::vec3 texCoord1 = this->texCoord[this->getTetra(tetraIdx).pointsIdx[0]];
        glm::vec3 texCoord2 = this->texCoord[this->getTetra(tetraIdx).pointsIdx[1]];
        glm::vec3 texCoord3 = this->texCoord[this->getTetra(tetraIdx).pointsIdx[2]];
        glm::vec3 texCoord4 = this->texCoord[this->getTetra(tetraIdx).pointsIdx[3]];
        glm::vec3 coordInInitial = this->getTetra(tetraIdx).baryToCoord(baryCoordInDeformed, texCoord1, texCoord2, texCoord3, texCoord4);
        out = coordInInitial;
        return true;
    } else {
        return false;
    }
}

bool TetMesh::getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, const std::vector<bool>& visibilityMap, const glm::vec3& planePos, glm::vec3& res) const {
    std::cout << "Cast ray not implemented yet for Tetmesh" << std::endl;
    return false;
}

void TetMesh::loadMESH(std::string const &filename) {

    std::ifstream myfile(filename);

    if (!myfile.is_open())
        exit (EXIT_FAILURE);

    std::string meshString;
    unsigned int sizeV, sizeT, sizeTet, dimension;

    myfile >> meshString;
    while (meshString.find("Dimension")==std::string::npos)
        myfile >> meshString;

    myfile>> dimension;
    std::cout << meshString << " " << dimension << std::endl;
    while (meshString.find("Vertices")==std::string::npos)
        myfile >> meshString;

    myfile >> sizeV;
    std::cout << meshString << " " << sizeV << std::endl;

    int s;
    for (unsigned int i = 0; i < sizeV; i++) {
        double p[3];
        for (unsigned int j = 0; j < 3; j++)
            myfile >> p[j];

        this->vertices.push_back(glm::vec3(static_cast<float>(p[0]), static_cast<float>(p[1]), static_cast<float>(p[2])));
        //std::cout << this->vertices.back() << std::endl;
        myfile >> s;
    }

    while (meshString.find("Triangles")==std::string::npos)
        myfile >> meshString;

    myfile >> sizeT;
    std::cout << meshString << " " << sizeT << std::endl;
    for (unsigned int i = 0; i < sizeT; i++) {
        unsigned int v[3];
        for (unsigned int j = 0; j < 3; j++)
            myfile >> v[j];

        myfile >> s;

        //triangles.push_back(Triangle(v[0]-1, v[1]-1, v[2]-1, s));
    }

    if( dimension == 3 ){
        while (meshString.find("Tetrahedra")==std::string::npos)
            myfile >> meshString;

        myfile >> sizeTet;
        std::cout << meshString << " " << sizeTet << std::endl;
        for (unsigned int i = 0; i < sizeTet; i++) {
            unsigned int v[4];
            for (unsigned int j = 0; j < 4; j++)
                myfile >> v[j];
            myfile >> s;
            mesh.push_back(Tetrahedron(&this->vertices[v[0]-1], &this->vertices[v[1]-1], &this->vertices[v[2]-1], &this->vertices[v[3]-1]));
            this->mesh.back().pointsIdx[0] = v[0]-1;
            this->mesh.back().pointsIdx[1] = v[1]-1;
            this->mesh.back().pointsIdx[2] = v[2]-1;
            this->mesh.back().pointsIdx[3] = v[3]-1;
        }
    }
    myfile.close ();

    std::cout << "Points: " << this->vertices.size() << std::endl;
    std::cout << "Tetrahedron: " << this->mesh.size() << std::endl;

    this->history = new History(this->vertices, this->coordinate_system);

    this->updatebbox();
    this->computeNeighborhood();
    this->computeNormals();

    for(int i = 0; i < this->vertices.size(); ++i) {
        this->texCoord.push_back(this->vertices[i]/this->getDimensions());
    }    
}

void TetMesh::sortTet(const glm::vec3& cameraOrigin, std::vector<std::pair<int, float>>& idxDepthMap) {
    idxDepthMap.clear();
    idxDepthMap = std::vector<std::pair<int, float>>(this->mesh.size());

    glm::vec3 centroid;
    for(int i = 0; i < this->mesh.size(); ++i) {
        this->mesh[i].getCentroid(centroid);
        idxDepthMap[i] = std::make_pair(i, glm::distance(centroid, cameraOrigin));
    }

    std::sort(idxDepthMap.begin(), idxDepthMap.end(),
        [](const std::pair<int, float> & a, const std::pair<int, float> & b) -> bool {
        return a.second < b.second;
    });

}
