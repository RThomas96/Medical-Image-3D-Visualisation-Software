
#include "info_pannel.hpp"
#include "../scene.hpp"

void InfoPannel::connect(Scene * scene) {
    QObject::connect(scene, &Scene::selectedPointChanged, this, &InfoPannel::updatePointInfo);
}
