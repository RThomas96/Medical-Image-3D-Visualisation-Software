#ifndef CELLINFO_H
#define CELLINFO_H

class CellInfo
{
public:
    CellInfo():index(0), cage_inlier(true), distortion_laplacian(0.) {
       for(int i = 0 ; i < 3; i++ )
            basis_def.push_back(glm::vec3(0.,0.,0.));
       for(int i = 0; i < 4; ++i)
           neighbors[i] = -1;
    }

    ~CellInfo(){}

    int index;
    bool cage_inlier;// Les points dans la cage
    std::vector<glm::vec3> basis_def;// Résultat du LRI pour les inconnue, nouvelle base, trois vecteurs, sino c'est la base normal
    float distortion_laplacian;// Pour le calcul de l'erreur (pas sur)
    int neighbors[4];
};

#endif // CELLINFO_H

