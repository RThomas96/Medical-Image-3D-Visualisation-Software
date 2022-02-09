#include <catch2/catch_test_macros.hpp>
#include "../grid/include/grid.hpp"
#include "../grid/include/surface_mesh.hpp"
#include "../grid/include/mathematics.h"
#include "BitmapPlusPlus.hpp"
#include <limits>

#include <numeric>
#include <glm/gtx/string_cast.hpp> 

void saveProfile(std::vector<std::vector<uint16_t>> profile, std::string name) {
    bmp::Bitmap image(profile.size(), profile[0].size());
    for(int i = 0; i < profile.size(); ++i) {
        for(int j = 0; j < profile[i].size(); ++j) {
            float value = static_cast<float>(float(profile[i][j]) / float(std::numeric_limits<uint8_t>::max())) * 255.;
            image.Set(i, j, bmp::Pixel(value, value, value));
        }
    }
    image.Save(name.c_str());
}

void saveDistance(std::vector<std::vector<long>> profile, std::string name) {
    bmp::Bitmap image(profile.size(), profile[0].size());
    for(int i = 0; i < profile.size(); ++i) {
        for(int j = 0; j < profile[i].size(); ++j) {
            float value = (profile[i][j] / 1300500.)*255.;
            //std::cout << value << " ";
            image.Set(i, j, bmp::Pixel(value, value, value));
        }
        //std::cout << std::endl;
    }
    image.Save(name.c_str());
}


class ICPMesh : public SurfaceMesh {

public:
    glm::mat4 originalTransformation;
    std::vector<glm::vec3> originalPoints;
    std::vector<glm::vec3> correspondence;
    std::vector<float> weights;

    ICPMesh(const std::vector<glm::vec3>& vertices, const std::vector<Triangle2>& triangles): SurfaceMesh(vertices, triangles) {
        this->originalTransformation = this->transformation;
        for(int i = 0; i < this->getNbVertices(); ++i) {
            this->correspondence.push_back(glm::vec3(0., 0., 0.));
            this->weights.push_back(1.f);
            this->originalPoints.push_back(this->getWorldVertice(i));
        }
    }

    ICPMesh(std::string const &filename) : SurfaceMesh(filename) {
        this->originalTransformation = this->transformation;
        for(int i = 0; i < this->getNbVertices(); ++i) {
            this->correspondence.push_back(glm::vec3(0., 0., 0.));
            this->weights.push_back(1.f);
            this->originalPoints.push_back(this->getWorldVertice(i));
        }
    }

    void setCorrespondence(glm::vec3 p, int i) {
        this->correspondence[i] = p;
    }

    glm::vec3 getCorrespondence(int i) const {
        return this->correspondence[i];
    }

    void setWeight(float p, int i) {
        this->weights[i] = p;
    }

    float getWeight(int i) const {
        return this->weights[i];
    }

};


class ICP {

public:
    unsigned int Ni=30,No=30,S=20;
    //unsigned int Ni=25,No=25,S=10;
    //float l=1.;
    float l=0.25;

    int nbIt = 0;

    glm::mat3 rotation;
    glm::vec3 origin;

    ICPMesh * surface;

    Grid * src;
    Grid * target;

    std::vector<std::vector<uint16_t>> sourceProf;

    ICP(Grid * src, Grid * target, ICPMesh * surface): src(src), target(target), surface(surface) {
        this->rotation = glm::mat3(1.f);
        this->origin = glm::vec3(0., 0., 0.);
    }

    ICP(Grid * src, Grid * target, std::string const &filename): src(src), target(target), surface(new ICPMesh(filename)) {

        //this->surface->setOrigin(glm::vec3(130., 50., 130.));
        //this->surface->setOrigin(glm::vec3(90., 50., 130.));
        //this->surface->setScale(20.);
        //this->surface->originalTransformation = this->surface->transformation;
        //for(int i = 0; i < this->surface->getNbVertices(); ++i) {
        //    this->surface->originalPoints[i] = this->surface->getWorldVertice(i);
        //}

        this->rotation = glm::mat3(1.f);
        this->origin = glm::vec3(0., 0., 0.);
    }

    void computeCorrespondences(ICPMesh& mesh, const std::vector<std::vector<long>> &dist, const float l) {
        unsigned int nbpoints=dist[0].size();
        unsigned int S=dist.size()/2;

        float ratio = float(Ni) / float(Ni+No);
        int NiIdxInS = std::floor(ratio*Ni);

        std::vector<std::vector<long>> r_dist(dist[0].size(), std::vector<long>(dist.size(), long(0)));
        for(int i = 0; i < dist.size(); ++i) {
            for(int j = 0; j < dist[i].size(); ++j) {
                r_dist[j][i] = dist[i][j];
            }
        }

        for(unsigned int i=0;i<nbpoints;i++) {
            glm::vec3 p = mesh.getWorldVertice(i);
            glm::vec3 normal = mesh.getWorldVerticeNormal(i);

            auto minptr = std::min_element(r_dist[i].begin(), r_dist[i].end());
            int idxMin = std::distance(r_dist[i].begin(), minptr);

            float distance  = l * (idxMin - int(S));
            
            p += distance * normal;

            mesh.setCorrespondence(p, i);
        }

    }
    
    std::vector<std::vector<uint16_t>> computeProfiles(const ICPMesh& mesh, const Grid& img, const unsigned int Ni, const unsigned int No, const float l, bool print = false) {

        std::vector<std::vector<uint16_t>> prof(Ni+No, std::vector<uint16_t>(mesh.getNbVertices(), 0.));
    
        for(int ptIdx = 0; ptIdx < mesh.getNbVertices(); ++ptIdx) {
            glm::vec3 currentPoint = mesh.getWorldVertice(ptIdx);
            glm::vec3 currentNormal = mesh.getWorldVerticeNormal(ptIdx);

            for(int i = 0; i < 3; ++i) {
                currentPoint[i] = std::roundf(currentPoint[i] * 1000) / 1000.0;
                currentNormal[i] = std::roundf(currentNormal[i] * 1000) / 1000.0;
            }
    
            for(int it = 0; it < No; ++it) {
                float offset = (float(it) * l) + 0.0001;
                prof[Ni+it][ptIdx] = img.getValueFromWorldPoint(currentPoint + offset * currentNormal, InterpolationMethod::Cubic);
                if(print)
                    std::cout << glm::to_string(currentPoint + offset * currentNormal) << std::endl;
            }
            for(int it = 0; it < Ni; ++it) {
                float offset = (float(it) * l) + 0.0001;
                prof[Ni-1-it][ptIdx] = img.getValueFromWorldPoint(currentPoint - offset * currentNormal, InterpolationMethod::Cubic);
                if(print)
                    std::cout << glm::to_string(currentPoint + offset * currentNormal) << std::endl;
            }
        }
        return prof;
    }

    std::vector<std::vector<long>> computeDistance(const std::vector<std::vector<uint16_t>>& sourceProf, const std::vector<std::vector<uint16_t>>& targetProf) {

        unsigned int nbpoints=sourceProf[0].size();
        unsigned int N=sourceProf.size();
        unsigned int S=(targetProf.size()-sourceProf.size())/2;

        std::vector<std::vector<long>> dist(2*S, std::vector<long>(nbpoints, 0));

        std::vector<long> cumul(nbpoints, 0);
        int insertColumnIndex = 0;
        for(int offset = 0; offset < S*2; ++offset) {
            std::fill(cumul.begin(), cumul.end(), 0);
            for(int ptIdx = 0; ptIdx < nbpoints; ++ptIdx) {
                for(int srcIdx = 0; srcIdx < N; ++srcIdx) {
                    long value = static_cast<long>(targetProf[srcIdx+offset][ptIdx]) - static_cast<long>(sourceProf[srcIdx][ptIdx]);
                    cumul[ptIdx] += value * value;
                }
            }
            for(int i = 0; i < nbpoints; ++i) {
                dist[insertColumnIndex][i] = cumul[i];
            }
            insertColumnIndex += 1;
        }

        return dist;
    }

    void Registration(glm::mat3& A,  glm::vec3& t,const ICPMesh& mesh) {
        glm::vec3 c0 = glm::vec3(0., 0., 0.);
        glm::vec3 c = glm::vec3(0., 0., 0.);
        float N = 0.;
        for(unsigned int i=0;i<mesh.getNbVertices();i++)
        {
            c0 += mesh.getWeight(i)*mesh.originalPoints[i];
            c  += mesh.getWeight(i)*mesh.getCorrespondence(i);
            N  += mesh.getWeight(i);
        }

        c0 /= N; 
        c  /= N;

        glm::mat3 K(0.f);
        float sx = 0;
        for(unsigned int i=0;i<mesh.getNbVertices();i++)
        {
            glm::vec3 p0 = mesh.originalPoints[i];
            p0 -= c0;

            glm::vec3 p = mesh.getCorrespondence(i);
            p -= c;

            for(unsigned int j=0;j<3;j++) {
                sx+=mesh.getWeight(i)*p0[j]*p0[j]; 
                for(unsigned int k=0;k<3;k++) {
                    K[j][k]+=mesh.getWeight(i)*p[j]*p0[k];
                } 
            }
        }

        float rawK[3][3];
        float rawA[3][3];

        fromGlmToRaw(K, rawK);
        fromGlmToRaw(A, rawA);

        ClosestRigid(rawK, rawA);

        fromRawToGlm(rawK, K);
        fromRawToGlm(rawA, A);

        t = A * c0;
        t = c - t;
    }

    void iteration(bool print=false) {
        if(this->sourceProf.size() <= 0) {
            this->sourceProf = computeProfiles(*this->surface, *this->src, Ni, No, l);
            return;
        }

        std::vector<std::vector<uint16_t>> targetProf = computeProfiles(*this->surface, *this->target, Ni+S, No+S, l, print);
        std::vector<std::vector<long>> dist = computeDistance(this->sourceProf,targetProf);
        computeCorrespondences(*this->surface,dist,l);
        Registration(this->rotation, this->origin, *this->surface);

        glm::mat4 transformation(1.f);
        for(int i = 0; i < 3; ++i) {
            transformation[3][i] = this->origin[i];
            for(int j = 0; j < 3; ++j)
                transformation[i][j] = this->rotation[i][j];
        }
        this->surface->transformation = transformation * this->surface->originalTransformation;

        if(print)
            saveProfile(targetProf, (std::string("targetProf") + std::to_string(nbIt) + std::string(".bmp")).c_str());

        nbIt += 1;
    }
};

TEST_CASE("Check one iteration on tilted triangle", "[ICP]") {
    Grid src(glm::vec3(11., 11., 11.));
    Grid trg(glm::vec3(11., 11., 11.));

    ICPMesh icpMesh(std::vector<glm::vec3>{glm::vec3(0., 0., 0.), glm::vec3(1., 0., 0.), glm::vec3(0., 1., 0.)}, 
                    std::vector<Triangle2>{Triangle2(0, 1, 2)});

    std::vector<uint16_t> sliceFilled(11.*11., 255);
    src.sampler.cache->img.get_shared_slice(5.).assign(sliceFilled.data(), 11., 11., 1.);
    trg.sampler.cache->img.get_shared_slice(8.).assign(sliceFilled.data(), 11., 11., 1.);
    //trg.rotate(glm::mat3{});
    //trg.translate(glm::vec3(1., 2., 0.));

    ICP icp(&src, &trg, &icpMesh);
    icp.Ni = 1;
    icp.No = 6;
    icp.S  = 6;
    icp.l  = 1;

    icp.iteration();
    icp.iteration();

    CHECK(glm::distance(icpMesh.getWorldVertice(0), glm::vec3(0., 0., 3.)) < 0.00001);
    CHECK(glm::distance(icpMesh.getWorldVertice(1), glm::vec3(1., 0., 3.)) < 0.00001);
    CHECK(glm::distance(icpMesh.getWorldVertice(2), glm::vec3(0., 1., 3.)) < 0.00001);

    CHECK(glm::distance(icpMesh.originalPoints[0], glm::vec3(0., 0., 0.)) < 0.00001);
    CHECK(glm::distance(icpMesh.originalPoints[1], glm::vec3(1., 0., 0.)) < 0.00001);
    CHECK(glm::distance(icpMesh.originalPoints[2], glm::vec3(0., 1., 0.)) < 0.00001);
}

TEST_CASE("Check one iteration on triangle", "[ICP]") {
    Grid src(glm::vec3(11., 11., 11.));
    Grid trg(glm::vec3(11., 11., 11.));

    ICPMesh icpMesh(std::vector<glm::vec3>{glm::vec3(0., 0., 0.), glm::vec3(1., 0., 0.), glm::vec3(0., 1., 0.)}, 
                    std::vector<Triangle2>{Triangle2(0, 1, 2)});

    std::vector<uint16_t> sliceFilled(11.*11., 255);
    src.sampler.cache->img.get_shared_slice(5.).assign(sliceFilled.data(), 11., 11., 1.);
    trg.sampler.cache->img.get_shared_slice(8.).assign(sliceFilled.data(), 11., 11., 1.);

    ICP icp(&src, &trg, &icpMesh);
    icp.Ni = 1;
    icp.No = 6;
    icp.S  = 6;
    icp.l  = 1;

    icp.iteration();
    icp.iteration();

    CHECK(glm::distance(icpMesh.getWorldVertice(0), glm::vec3(0., 0., 3.)) < 0.00001);
    CHECK(glm::distance(icpMesh.getWorldVertice(1), glm::vec3(1., 0., 3.)) < 0.00001);
    CHECK(glm::distance(icpMesh.getWorldVertice(2), glm::vec3(0., 1., 3.)) < 0.00001);

    CHECK(glm::distance(icpMesh.originalPoints[0], glm::vec3(0., 0., 0.)) < 0.00001);
    CHECK(glm::distance(icpMesh.originalPoints[1], glm::vec3(1., 0., 0.)) < 0.00001);
    CHECK(glm::distance(icpMesh.originalPoints[2], glm::vec3(0., 1., 0.)) < 0.00001);

    icp.iteration();
    icp.iteration();
    icp.iteration();
    icp.iteration();
    icp.iteration();

    CHECK(glm::distance(icpMesh.getWorldVertice(0), glm::vec3(0., 0., 3.)) < 0.00001);
    CHECK(glm::distance(icpMesh.getWorldVertice(1), glm::vec3(1., 0., 3.)) < 0.00001);
    CHECK(glm::distance(icpMesh.getWorldVertice(2), glm::vec3(0., 1., 3.)) < 0.00001);

    CHECK(glm::distance(icpMesh.originalPoints[0], glm::vec3(0., 0., 0.)) < 0.00001);
    CHECK(glm::distance(icpMesh.originalPoints[1], glm::vec3(1., 0., 0.)) < 0.00001);
    CHECK(glm::distance(icpMesh.originalPoints[2], glm::vec3(0., 1., 0.)) < 0.00001);
}

TEST_CASE("Check full pipeline", "[ICP]") {

    Grid src(glm::vec3(11., 11., 11.));
    Grid trg(glm::vec3(11., 11., 11.));

    std::vector<uint16_t> sliceFilled(11.*11., 255);
    src.sampler.cache->img.get_shared_slice(8.).assign(sliceFilled.data(), 11., 11., 1.);
    trg.sampler.cache->img.get_shared_slice(5.).assign(sliceFilled.data(), 11., 11., 1.);

    CHECK(src.getValueFromWorldPoint(glm::vec3(0., 0., 0.)) ==   0);
    CHECK(src.getValueFromWorldPoint(glm::vec3(0., 0., 8.)) == 255);
    CHECK(src.getValueFromWorldPoint(glm::vec3(5., 5., 8.)) == 255);
    
    std::vector<glm::vec3> vertices{glm::vec3(0., 0., 0.), glm::vec3(1., 0., 0.), glm::vec3(0., 1., 0.)};
    std::vector<Triangle2> triangles{Triangle2(0, 1, 2)};
    ICPMesh icpMesh(vertices, triangles);

    CHECK(glm::distance(icpMesh.verticesNormals[0], glm::vec3(0., 0., 1.)) < 0.000001);
    CHECK(glm::distance(icpMesh.verticesNormals[1], glm::vec3(0., 0., 1.)) < 0.000001);
    CHECK(glm::distance(icpMesh.verticesNormals[2], glm::vec3(0., 0., 1.)) < 0.000001);

    ICP icp(&src, &trg, &icpMesh);
   
    // Check compute profile on aligned triangle
    std::vector<std::vector<uint16_t>> targetProf = icp.computeProfiles(icpMesh, trg, 0, 6, 1);
    CHECK(targetProf[5][0] == 255);
    CHECK(targetProf[5][1] == 255);
    CHECK(targetProf[5][2] == 255);

    icpMesh.setOrigin(glm::vec3(0., 0., 1.));
    targetProf = icp.computeProfiles(icpMesh, trg, 0, 6, 1);
    CHECK(targetProf[4][0] == 255);
    CHECK(targetProf[4][1] == 255);
    CHECK(targetProf[4][2] == 255);

    icpMesh.setOrigin(glm::vec3(0., 0., 3.));
    targetProf = icp.computeProfiles(icpMesh, trg, 0, 6, 1);
    CHECK(targetProf[2][0] == 255);
    CHECK(targetProf[2][1] == 255);
    CHECK(targetProf[2][2] == 255);

    // Check compute distance on aligned triangle
    // Reset
    src.sampler.cache->reset();
    trg.sampler.cache->reset();

    icpMesh.setOrigin(glm::vec3(0., 0., 0.));

    src.sampler.cache->img.get_shared_slice(5.).assign(sliceFilled.data(), 11., 11., 1.);
    trg.sampler.cache->img.get_shared_slice(8.).assign(sliceFilled.data(), 11., 11., 1.);

    std::vector<std::vector<uint16_t>> srcProf = icp.computeProfiles(icpMesh, src, 0, 6, 1);
    CHECK(srcProf[5][0] == 255);
    CHECK(srcProf[5][1] == 255);
    CHECK(srcProf[5][2] == 255);

    targetProf = icp.computeProfiles(icpMesh, trg, 6, 12, 1);
    CHECK(targetProf[14][0] == 255);
    CHECK(targetProf[14][1] == 255);
    CHECK(targetProf[14][2] == 255);

    std::vector<std::vector<long>> dist = icp.computeDistance(srcProf, targetProf);
    saveDistance(dist, std::string("distance.bmp"));
    CHECK(dist[6][0] == 65025);
    CHECK(dist[7][0] == 65025);
    CHECK(dist[8][0] == 65025);
    CHECK(dist[9][0] == 0);
    CHECK(dist[10][0] == 130050);
    CHECK(dist[11][0] == 130050);

    icp.computeCorrespondences(icpMesh, dist, 1);

    CHECK(glm::distance(icpMesh.correspondence[0], icpMesh.originalPoints[0]) == 3.);
    CHECK(glm::distance(icpMesh.correspondence[1], icpMesh.originalPoints[1]) == 3.);
    CHECK(glm::distance(icpMesh.correspondence[2], icpMesh.originalPoints[2]) == 3.);

    glm::vec3 origin;
    glm::mat3 rotation;
    icp.Registration(rotation, origin, icpMesh);

    glm::mat4 transformation(1.f);
    for(int i = 0; i < 3; ++i) {
        transformation[3][i] = origin[i];
        for(int j = 0; j < 3; ++j)
            transformation[i][j] = rotation[i][j];
    }

    icpMesh.transformation = transformation;

    CHECK(glm::distance(icpMesh.getWorldVertice(0), glm::vec3(0., 0., 3.)) < 0.00001);
    CHECK(glm::distance(icpMesh.getWorldVertice(1), glm::vec3(1., 0., 3.)) < 0.00001);
    CHECK(glm::distance(icpMesh.getWorldVertice(2), glm::vec3(0., 1., 3.)) < 0.00001);

    CHECK(glm::distance(icpMesh.originalPoints[0], glm::vec3(0., 0., 0.)) < 0.00001);
    CHECK(glm::distance(icpMesh.originalPoints[1], glm::vec3(1., 0., 0.)) < 0.00001);
    CHECK(glm::distance(icpMesh.originalPoints[2], glm::vec3(0., 1., 0.)) < 0.00001);
}

TEST_CASE("Check cube", "[ICP]") {

    Grid src(std::vector<std::string>{"../../tests/data/ICP_cube.tif"}, 1.);
    src.setOrigin(glm::vec3(-10., -10., -10.));

    Grid target(std::vector<std::string>{"../../tests/data/ICP_cube.tif"}, 1.);
    target.setOrigin(glm::vec3(-7., -10., -10.));

    ICP icp(&src, &target, "../../tests/data/ICP_simple_cube.off");
    icp.Ni = 12;
    icp.No = 12;
    icp.S  = 12;
    icp.l  = 0.5;

    icp.iteration();

    for(int i = 0; i < 9; ++i) {
        icp.iteration();
        std::cout << "--------------" << std::endl;
        std::cout << "STEP" << i << std::endl;
        std::cout << "--------------" << std::endl;
        for(int i = 0; i < icp.surface->getNbVertices(); ++i) {
            std::cout << glm::to_string(icp.surface->getWorldVertice(i)) << std::endl;
        }
        std::cout << "---" << std::endl << std::endl;
    }
    std::cout << "---GOAL---" << std::endl << std::endl;
    for(int i = 0; i < icp.surface->getNbVertices(); ++i) {
        std::cout << glm::to_string(icp.surface->originalPoints[i] + glm::vec3(3., 0., 0.)) << std::endl;
    }
}
