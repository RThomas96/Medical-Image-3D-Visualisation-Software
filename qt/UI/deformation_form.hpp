#ifndef QT_DEFORMATION_FORM
#define QT_DEFORMATION_FORM

#include <glm/glm.hpp>
#include "form.hpp"

class Scene;

class DeformationForm : Form {
    Q_OBJECT

public:

    std::vector<glm::vec3> origins;
    std::vector<glm::vec3> results;
    DeformationForm(Scene * scene, QWidget *parent = nullptr):Form(parent){init();connect(scene);}

public slots:

    void init();
    void update(Scene * scene);
    void show();
    void extractPointsFromText(std::vector<glm::vec3>& points);
    void convertPoints(Scene * scene);
    std::string getFromGridName();
    std::string getToGridName();
    void connect(Scene * scene);
};

#endif
