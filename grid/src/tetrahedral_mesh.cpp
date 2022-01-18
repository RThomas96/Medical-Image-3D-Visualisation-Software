#include "../include/tetrahedral_mesh.hpp"
#include <map>


// This function make the link between a face of a tetrahedron and its points
// This ensure a stable way to iterate throught faces of a tetrahedron
// These are global variable as it is common to all tetrahedron
// You can also see the faceIdx as the point index which in the opposite side of the face
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

int Tetrahedron::getPointIndex(int faceIdx, int ptIdxInFace) {
    return this->pointsIdx[getIdxOfPtInFace(faceIdx, ptIdxInFace)];
}

std::vector<glm::vec3*> TetMesh::insertCubeIntoPtGrid(std::vector<glm::vec3> cubePts, glm::vec3 indices, std::vector<glm::vec3>& ptGrid, std::vector<int>& ptIndices) {
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

    ptGrid[ptIndices[0]] = cubePts[0];
    ptGrid[ptIndices[1]] = cubePts[1];
    ptGrid[ptIndices[2]] = cubePts[2];
    ptGrid[ptIndices[3]] = cubePts[3];

    ptGrid[ptIndices[4]] = cubePts[4];
    ptGrid[ptIndices[5]] = cubePts[5];
    ptGrid[ptIndices[6]] = cubePts[6];
    ptGrid[ptIndices[7]] = cubePts[7];

    glm::vec3 tetMeshSize = bbMax - bbMin;
    texCoordGrid[ptIndices[0]] = cubePts[0]/tetMeshSize;
    texCoordGrid[ptIndices[1]] = cubePts[1]/tetMeshSize;
    texCoordGrid[ptIndices[2]] = cubePts[2]/tetMeshSize;
    texCoordGrid[ptIndices[3]] = cubePts[3]/tetMeshSize;

    texCoordGrid[ptIndices[4]] = cubePts[4]/tetMeshSize;
    texCoordGrid[ptIndices[5]] = cubePts[5]/tetMeshSize;
    texCoordGrid[ptIndices[6]] = cubePts[6]/tetMeshSize;
    texCoordGrid[ptIndices[7]] = cubePts[7]/tetMeshSize;

    std::vector<glm::vec3*> res;
    res.push_back(&ptGrid[ptIndices[0]]);
    res.push_back(&ptGrid[ptIndices[1]]);
    res.push_back(&ptGrid[ptIndices[2]]);
    res.push_back(&ptGrid[ptIndices[3]]);

    res.push_back(&ptGrid[ptIndices[4]]);
    res.push_back(&ptGrid[ptIndices[5]]);
    res.push_back(&ptGrid[ptIndices[6]]);
    res.push_back(&ptGrid[ptIndices[7]]);
    
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

void Tetrahedron::computeNormals() {
    for(int faceIdx = 0; faceIdx < 4; ++faceIdx) {
        glm::vec3 n1   = *this->points[getIdxOfPtInFace(faceIdx, 1)] - *this->points[getIdxOfPtInFace(faceIdx, 0)];
        glm::vec3 n2   = *this->points[getIdxOfPtInFace(faceIdx, 2)] - *this->points[getIdxOfPtInFace(faceIdx, 0)];
        glm::vec4 norm = glm::vec4(glm::normalize(glm::cross(glm::vec3(n1), glm::vec3(n2))), 1.);
        // Put inverse of dot with opposing vertex in norm.w :
        glm::vec4 v1			  = glm::vec4(*this->points[faceIdx] - *this->points[(faceIdx + 1) % 4], 1.);
        glm::vec4::value_type val = 1. / glm::dot(v1, norm);
        norm.w					  = val;
        this->normals[faceIdx] = norm;
    }
}

void TetMesh::buildGrid(const glm::vec3& nbCube, const glm::vec3& sizeCube, const glm::vec3& origin) {
    if(!this->isEmpty())
        throw std::runtime_error("Error: build grid cannot be used on already constructed mesh.");
    // Reason: nbTetra, bbMin and bbMax aren't correctly updated when build grid is used on already constructed mesh
    this->nbTetra += nbCube;
    this->bbMin = origin;
    this->bbMax = origin+nbCube * sizeCube;

    // We add +1 here because we stock points, and there one more point than cube as it need a final point
    ptGrid = std::vector<glm::vec3>((this->nbTetra[0]+1)*(this->nbTetra[1]+1)*(this->nbTetra[2]+1), glm::vec3(0., 0., 0.));
    texCoordGrid = std::vector<glm::vec3>((this->nbTetra[0]+1)*(this->nbTetra[1]+1)*(this->nbTetra[2]+1), glm::vec3(0., 0., 0.));

    std::vector<int> ptIndices(8, -1);
    for(int k = 0; k < nbCube[2]; ++k) {
        for(int j = 0; j < nbCube[1]; ++j) {
            for(int i = 0; i < nbCube[0]; ++i) {
                glm::vec3 offset = glm::vec3(i*sizeCube[0], j*sizeCube[1], k*sizeCube[2]);
                std::vector<glm::vec3> cubePts = buildCube(origin+offset, sizeCube);
                std::vector<glm::vec3*> cubePtsAdress = insertCubeIntoPtGrid(cubePts, glm::vec3(i, j, k), this->ptGrid, ptIndices);
                this->decomposeAndAddCube(cubePtsAdress, ptIndices);
            }
        }
    }
    this->computeNeighborhood();
    this->computeNormals();
}

void TetMesh::movePoint(const glm::vec3& indices, const glm::vec3& position) {
    glm::vec3& p = this->ptGrid[this->from3DTo1D(indices)];
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

glm::vec3 TetMesh::getDimensions() const {
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
                //this->ptGrid[i][j][k] = pts[l];
                this->ptGrid[this->from3DTo1D(glm::vec3(i, j, k))] = pts[l];
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
    for(int tetIdx; tetIdx < this->mesh.size(); ++tetIdx) {
        this->mesh[tetIdx].computeNormals();
    }
}

/* Unit test */
