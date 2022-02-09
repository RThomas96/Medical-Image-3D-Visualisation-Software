#include <catch2/catch_test_macros.hpp>
#include "../grid/include/grid.hpp"
#include "../grid/include/surface_mesh.hpp"
#include "../grid/include/mathematics.h"
#include "BitmapPlusPlus.hpp"
#include <limits>

#include <numeric>
#include <glm/gtx/string_cast.hpp> 

class ICPMesh : public SurfaceMesh {

public:
    glm::mat4 originalTransformation;
    std::vector<glm::vec3> originalPoints;
    std::vector<glm::vec3> correspondence;
    std::vector<float> weights;

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

    glm::mat3 rotation;
    glm::vec3 origin;

    ICPMesh * surface;

    Grid * src;
    Grid * target;

    std::vector<std::vector<uint16_t>> sourceProf;

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

    void computeCorrespondences(ICPMesh& mesh, const std::vector<std::vector<float>> &dist,const float l) {
        unsigned int nbpoints=dist[0].size();
        unsigned int S=dist.size()/2;

        for(unsigned int i=0;i<nbpoints;i++)
        {
            //glm::vec3 p = (this->rotation * mesh.getWorldVertice(i)) + this->origin;
            glm::vec3 p = mesh.getWorldVertice(i);

            float min = 1000000;
            int idxMin = -1;
            for(int j = 0; j < 2*S; ++j) {
                if(dist[j][i] < min) {
                    min = dist[j][i];
                    idxMin = j;
                }
            }

            //glm::vec3 normal = (this->rotation * mesh.getWorldVerticeNormal(i)) + this->origin;
            glm::vec3 normal = mesh.getWorldVerticeNormal(i);

            if(idxMin < S)
                normal = -normal;
            
            p += l * std::abs(int(S)-idxMin) * normal;

            mesh.setCorrespondence(p, i);
        }

    }
    
    std::vector<std::vector<uint16_t>> computeProfiles(const ICPMesh& mesh, const Grid& img, const unsigned int Ni, const unsigned int No, const float l) {

        std::vector<std::vector<uint16_t>> prof(Ni+No, std::vector<uint16_t>(mesh.getNbVertices(), 0.));
    
        for(int ptIdx = 0; ptIdx < mesh.getNbVertices(); ++ptIdx) {
            //glm::vec3 currentPoint = (this->rotation * mesh.getWorldVertice(ptIdx)) + this->origin;
            glm::vec3 currentPoint = mesh.getWorldVertice(ptIdx);
            //std::cout << glm::to_string(currentPoint) << std::endl;
            
            //glm::vec3 currentNormal = (this->rotation * mesh.getWorldVerticeNormal(ptIdx)) + this->origin;
            glm::vec3 currentNormal = mesh.getWorldVerticeNormal(ptIdx);
    
            for(float it = 0; it < No; ++it) {
                float offset = (it+0.0001) * l;
                prof[Ni+it][ptIdx] = img.getValueFromWorldPoint(currentPoint + offset * currentNormal);
            }
            currentNormal = -currentNormal;
            for(float it = 0; it < Ni; ++it) {
                float offset = (it+0.0001) * l;
                prof[Ni-1-it][ptIdx] = img.getValueFromWorldPoint(currentPoint + offset * currentNormal);
            }
        }
        /***/
        //std::cout << "Ni = " << Ni << std::endl;
        //std::cout << "No = " << No << std::endl;
        //std::cout << "l  = " << l << std::endl;
        //std::cout << "[";
        for(int i = 0; i < prof.size(); ++i) {
            int sum = std::accumulate(prof[i].begin(), prof[i].end(), 0);
            //std::cout << sum << ", ";
        }
        //std::cout << "]";
        //std::cout << std::endl;
        /***/
        return prof;
    }

    std::vector<std::vector<float>> computeDistance(const std::vector<std::vector<uint16_t>>& sourceProf, const std::vector<std::vector<uint16_t>>& targetProf) {

        unsigned int nbpoints=sourceProf[0].size();
        unsigned int N=sourceProf.size();
        unsigned int S=(targetProf.size()-sourceProf.size())/2;

        std::vector<std::vector<float>> dist(2*S, std::vector<float>(nbpoints, 0.));

        int insertColumnIndex = 0;
        for(int offset = 0; offset < S*2; ++offset) {
            long cumul[nbpoints];
            for(int i = 0; i < nbpoints; ++i)
                cumul[i] = 0;

            for(int ptIdx = 0; ptIdx < nbpoints; ++ptIdx) {
                for(int srcIdx = 0; srcIdx < N; ++srcIdx) {
                    int16_t value = static_cast<int16_t>(targetProf[srcIdx+offset][ptIdx]) - static_cast<int16_t>(sourceProf[srcIdx][ptIdx]);
                    cumul[ptIdx] +=  static_cast<long>(value * value);
                }
            }
            for(int i = 0; i < nbpoints; ++i) {
                dist[insertColumnIndex][i] = static_cast<float>(cumul[i]);
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
        //std::cout << "N is " << N << std ::endl;
        c0 /=N; 
        c  /=N;
        //std::cout << glm::to_string(c0) << std::endl;
        //std::cout << glm::to_string(c) << std::endl;

        glm::mat3 Q;
        glm::mat3 K;
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
                    Q[j][k]+=mesh.getWeight(i)*p0[j]*p0[k];  
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


    void iteration() {
        // compute source profiles
        // mesh.updateNormals();
        if(this->sourceProf.size() <= 0) {
            this->sourceProf = computeProfiles(*this->surface, *this->src, Ni, No, l);
            return;
        }

        std::vector<std::vector<uint16_t>> targetProf = computeProfiles(*this->surface, *this->target, Ni+S, No+S, l);

        std::vector<std::vector<float>> dist = computeDistance(this->sourceProf,targetProf);

        computeCorrespondences(*this->surface,dist,l);

        Registration(this->rotation,this->origin,*this->surface);

        glm::mat4 transformation(1.f);
        for(int i = 0; i < 3; ++i) {
            transformation[3][i] = this->origin[i];
            for(int j = 0; j < 3; ++j)
                transformation[i][j] = this->rotation[i][j];
        }
        std::cout << glm::to_string(transformation) << std::endl;
        //std::cout << "Rotation" << std::endl;
        //std::cout << glm::to_string(this->rotation) << std::endl;
        //std::cout << "Origin" << std::endl;
        //std::cout << glm::to_string(this->origin) << std::endl;
        //std::cout << "Transfo global" << std::endl;
        //std::cout << glm::to_string(transformation) << std::endl;
        //std::cout << "Multiplier par" << std::endl;
        //std::cout << glm::to_string(this->surface->transformation) << std::endl;
        this->surface->transformation = transformation * this->surface->originalTransformation;
        //std::cout << "Donne" << std::endl;
        //std::cout << glm::to_string(this->surface->transformation) << std::endl;
    }
};

void saveProfile(std::vector<std::vector<uint16_t>> profile, std::string name) {
    bmp::Bitmap image(profile.size(), profile[0].size());
    for(int i = 0; i < profile.size(); ++i) {
        for(int j = 0; j < profile[i].size(); ++j) {
            float value = static_cast<float>(profile[i][j] / std::numeric_limits<uint8_t>::max()) * 255.;
            image.Set(i, j, bmp::Pixel(value, value, value));
        }
    }
    image.Save(name.c_str());
}

void saveDistance(std::vector<std::vector<float>> profile, std::string name) {
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

TEST_CASE("Check compute profile", "[ICP]") {

    float offset = 5.;

    float l=1.;
    
    float ratio = 1/l;
    unsigned int No=11*ratio,Ni=No,S=offset+1.;

    l += 0.01;


    Grid src(std::vector<std::string>{"../../tests/data/ICP_cube.tif"}, 1.);
    src.setOrigin(glm::vec3(-10., -10., -10.));

    Grid target(std::vector<std::string>{"../../tests/data/ICP_cube.tif"}, 1.);
    target.setOrigin(glm::vec3(-10.+offset, -10., -10.));

    ICP icp(&src, &target, "../../tests/data/ICP_simple_cube.off");
    icp.Ni = Ni;
    icp.No = No;
    icp.S  = S;
    icp.l  = l;

    std::vector<std::vector<uint16_t>> sourceProf = icp.computeProfiles(*icp.surface, src, Ni, No, l);
    saveProfile(sourceProf, std::string("sourceProfile.bmp"));

    std::vector<std::vector<uint16_t>> targetProf = icp.computeProfiles(*icp.surface, target, Ni+S, No+S, l);
    saveProfile(targetProf, std::string("targetProfile.bmp"));

    std::vector<std::vector<float>> dist = icp.computeDistance(sourceProf, targetProf);
    saveDistance(dist, std::string("distance.bmp"));

    //icp.computeCorrespondences(*icp.surface, dist, 1.);

    //for(int i = 0; i < 20; ++i) {
    //    icp.iteration();
    //    std::cout << "--------------" << std::endl;
    //    std::cout << "STEP" << i << std::endl;
    //    std::cout << "--------------" << std::endl;
    //    for(int i = 0; i < icp.surface->getNbVertices(); ++i) {
    //        std::cout << glm::to_string(icp.surface->originalPoints[i]) << std::endl;
    //        std::cout << glm::to_string(icp.surface->getWorldVertice(i)) << std::endl;
    //        std::cout << "---" << std::endl << std::endl;
    //    }
    //}
}
