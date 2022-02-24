//! \defgroup algo Algorithm
//! \addtogroup algo
//! @{

class ICPMesh : public SurfaceMesh {

public:
    glm::mat4 originalTransformation;
    std::vector<glm::vec3> originalPoints;
    std::vector<glm::vec3> correspondence;
    std::vector<float> weights;

    ICPMesh(const std::vector<glm::vec3>& vertices, const std::vector<Triangle>& triangles): SurfaceMesh(vertices, triangles) {
        this->originalTransformation = this->getModelTransformation();
        for(int i = 0; i < this->getNbVertices(); ++i) {
            this->correspondence.push_back(glm::vec3(0., 0., 0.));
            this->weights.push_back(1.f);
            this->originalPoints.push_back(this->getWorldVertice(i));
        }
    }

    ICPMesh(std::string const &filename) : SurfaceMesh(filename) {
        this->originalTransformation = this->getModelTransformation();
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

    void getPoint0(float p[3], const unsigned int index) const {  for(unsigned int i=0;i<3;i++) p[i]=this->originalPoints[index][i]; }
    void getPoint(float p[3], const unsigned int index) const {  for(unsigned int i=0;i<3;i++) p[i]=this->getWorldVertice(index)[i]; }
    void getCorrespondence(float p[3], const unsigned int index) const {  for(unsigned int i=0;i<3;i++) p[i]=this->correspondence[index][i]; }
    float getWeight(const unsigned int index) const {  return this->weights[index]; }

};

class ICP {

public:
    //unsigned int Ni=20,No=20,S=10;
    //float l=0.4;

    unsigned int Ni=10,No=5,S=10;
    float l=0.02*100;

    int nbIt = 0;
    int metric = 1;

    glm::mat3 rotation;
    glm::vec3 origin;

    ICPMesh * surface;

    Grid * src;
    Grid * target;

    CImg<uint16_t> sourceProf;

    float A[3][3];
    float t[3];

    ICP(Grid * src, Grid * target, ICPMesh * surface): src(src), target(target), surface(surface) {
        this->rotation = glm::mat3(1.f);
        this->origin = glm::vec3(0., 0., 0.);
    }

    ICP(Grid * src, Grid * target, std::string const &filename): src(src), target(target), surface(new ICPMesh(filename)) {

        // Setup #1
        //this->surface->setScale(glm::vec3(35., 35., 35.));
        //this->surface->setOrigin(glm::vec3(137.871201, 115.300957, 142.449493));

        //this->src->setOrigin(glm::vec3(-50.-1.88574, -100.-1.13047, -60.-3.44723) + glm::vec3(110., 110., 110.));
 
        // Setup #2
        this->surface->setScale(glm::vec3(100., 100., 100.));
        //this->surface->setOrigin(glm::vec3(137.871201, 115.300957, 142.449493));
        //this->surface->setOrigin(((this->surface->bbMax - this->surface->bbMax)/2.f) + glm::vec3(110., 110., 110.));
        this->surface->setOrigin(glm::vec3(229.294800, -6.420403, 114.809021));

        this->src->setOrigin(glm::vec3(-50.-1.88574, -100.-1.13047, -60.-3.44723) + glm::vec3(110., 110., 110.));
        this->src->setScale(glm::vec3(1., 1., 3.));
        this->target->setOrigin(glm::vec3(-106.-2.3, -114.5-1., -50.-2.5) + glm::vec3(110., 110., 110.));
        this->target->setScale(glm::vec3(1.381, 1.29553, 3.90019));

        //this->rotation = glm::mat3(1.f);
        //this->origin = glm::vec3(0., 0., 0.);

        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                this->A[i][j] = 0;

        A[0][0] = 1;
        A[1][1] = 1;
        A[2][2] = 1;

        t[0] = 0;
        t[1] = 0;
        t[2] = 0;
    }

    void initialize() {
        this->surface->originalTransformation = this->surface->getModelTransformation();
        for(int i = 0; i < this->surface->getNbVertices(); ++i) {
            this->surface->correspondence.push_back(glm::vec3(0., 0., 0.));
            this->surface->weights.push_back(1.f);
            this->surface->originalPoints[i] = this->surface->getWorldVertice(i);
            //std::cout << glm::to_string(this->surface->originalPoints[i]);
        }
        this->sourceProf = computeProfiles(*this->surface, *this->src, Ni, No, l);
        this->sourceProf.print();
        this->sourceProf.save_bmp("srcProfil.bmp");
    }

    void computeCorrespondences(ICPMesh& mesh, const CImg<float> &dist, const float l, int metric) {
        unsigned int nbpoints=dist.height();
        unsigned int S=dist.width()/2;

        for(unsigned int ptIdx=0;ptIdx<nbpoints;ptIdx++) {
            glm::vec3 p = mesh.getWorldVertice(ptIdx);

            bool valueFound = (dist.get_row(ptIdx).mean() != 0.);
            if(valueFound) {
                std::vector<float> row(dist.get_shared_row(ptIdx).data(), dist.get_shared_row(ptIdx).data()+dist.width());
                int idx = 0;
                if(metric == 0)
                    idx = std::distance(row.begin(), std::min_element(row.begin(), row.end()));
                else
                    idx = std::distance(row.begin(), std::max_element(row.begin(), row.end()));

                float direction = 1.;
                if(idx < S)
                    direction = -1.;

                float distance = std::fabs(float(idx)-float(S)) * l;

                glm::vec3 currentNormal = mesh.getVerticeNormal(ptIdx);
                p += currentNormal * direction * distance;
            }

            mesh.setCorrespondence(p,ptIdx);
        }
    }
    
    CImg<uint16_t> computeProfiles(const ICPMesh& mesh, const Grid& img, const unsigned int Ni, const unsigned int No, const float l, bool print = false) {
        //for(int i = 0; i < 3; ++i) {
        //    currentPoint[i] = std::roundf(currentPoint[i] * 1000) / 1000.0;
        //    currentNormal[i] = std::roundf(currentNormal[i] * 1000) / 1000.0;
        //}
        CImg<uint16_t> prof(Ni+No,mesh.getNbVertices());
        prof.fill(0);

        for(int ptIdx = 0; ptIdx < mesh.getNbVertices(); ++ptIdx) {
            glm::vec3 currentPoint = mesh.getWorldVertice(ptIdx);
            glm::vec3 currentNormal = mesh.getVerticeNormal(ptIdx);

            float direction = -1.;
            for(int step = 0; step < Ni; ++step) {
                glm::vec3 newPoint = currentPoint + direction * float(step) * l * currentNormal;
                prof(Ni-step-1, ptIdx) = img.getValueFromWorldPoint(newPoint, InterpolationMethod::Linear);
            }

            direction = 1.;
            for(int step = 0; step < No; ++step) {
                glm::vec3 newPoint = currentPoint + direction * float(step) * l * currentNormal;
                prof(Ni+step, ptIdx) = img.getValueFromWorldPoint(newPoint, InterpolationMethod::Linear);
            }
        }

        //prof.display();
        prof.save_bmp("nextProfil.bmp");
        return prof;
    }

    // 0 = SSD
    CImg<float> computeDistance(const CImg<uint16_t>& sourceProf, const CImg<uint16_t>& targetProf, int metric) {

        unsigned int nbpoints=sourceProf.height();
        unsigned int N=sourceProf.width();
        unsigned int S=(targetProf.width()-sourceProf.width())/2;

        CImg<float> dist(2*S,nbpoints);
        dist.fill(0);

        if(metric==0) {
            for(int ptIdx = 0; ptIdx < nbpoints; ++ptIdx)
                for(int offset = 0; offset < 2*S; ++offset)
                    for(int step = 0; step < N; ++step)
                        dist(offset, ptIdx) += std::pow(float(sourceProf(step, ptIdx)) - float(targetProf(step+offset, ptIdx)), 2);
        } else {
            for(int ptIdx = 0; ptIdx < nbpoints; ++ptIdx) {
                bool meanToZero = false;
                double srcMean = sourceProf.get_row(ptIdx).mean();
                if(srcMean == 0) meanToZero = true;

                for(int offset = 0; offset < 2*S; ++offset) {
                    double trgMean = targetProf.get_row(ptIdx).get_columns(offset, offset+N).mean();
                    if(trgMean == 0) meanToZero = true;

                    if(meanToZero) {
                        dist(offset, ptIdx) = 0;
                    } else {
                        double subMult      = 0;
                        double subSquareSrc = 0;
                        double subSquareTrg = 0;
                        for(int step = 0; step < N; ++step) {
                            subMult += (double(sourceProf(step, ptIdx)) - srcMean) * (double(targetProf(offset+step, ptIdx)) - trgMean);
                            subSquareSrc += std::pow(double(sourceProf(step, ptIdx)) - srcMean, 2);
                            subSquareTrg += std::pow(double(targetProf(offset+step, ptIdx)) - trgMean, 2);
                        }

                        dist(offset, ptIdx) = subMult / std::sqrt(subSquareSrc * subSquareTrg);
                    }
                }
            }
        }
        //dist.display();
        dist.save_bmp("dist.bmp");
        return dist;
    }

    //void Registration(glm::mat3& A,  glm::vec3& t,const ICPMesh& mesh);
    void Registration(float A[3][3],  float t[3], const ICPMesh& mesh);

    void iteration(bool print=false) {
        this->surface->computeNormals();
        CImg<uint16_t> targetProf = computeProfiles(*this->surface, *this->target, Ni+S, No+S, l, print);
        CImg<float> dist = computeDistance(this->sourceProf,targetProf,this->metric);
        computeCorrespondences(*this->surface,dist,l,this->metric);
        Registration(this->A, this->t, *this->surface);

        glm::mat4 transformation(1.f);
        for(int i = 0; i < 3; ++i) {
            transformation[3][i] = -this->t[i];
            for(int j = 0; j < 3; ++j)
                transformation[i][j] = this->A[i][j];
        }
        this->surface->setTransformation(transformation * this->surface->originalTransformation);

        nbIt += 1;
    }
};

//! @}
