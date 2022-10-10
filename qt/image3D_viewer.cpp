#include "image3D_viewer.hpp"
#include "scene.hpp"

Raw3DImage::Raw3DImage(const glm::ivec3 imgSize, DrawableGrid * grid, QImage::Format format) {
    this->grid = grid;
    this->imgSize = imgSize;
    max = 0;
    this->format = format;
    for(int k = 0; k < imgSize[2]; ++k) {
        this->data.push_back(std::vector<uint16_t>(this->imgSize.x*this->imgSize.y, 0));
        this->images.push_back(QImage(this->imgSize.x, this->imgSize.y, format));
        this->images.back().fill(QColor(0, 0, 0));

        this->upToDate.push_back(false);
    }
}

void Raw3DImage::setSlice(const int& idx, const std::vector<uint16_t>& data) {
    this->data[idx] = data;
    this->updateMaxValue();
    this->upToDate[idx] = false;
}

void Raw3DImage::setImage(const std::vector<std::vector<uint16_t>>& data) {
    this->data = data;
    this->updateMaxValue();
    this->images.clear();
    this->upToDate.clear();
    for(int k = 0; k < imgSize[2]; ++k) {
        this->images.push_back(QImage(this->imgSize.x, this->imgSize.y, format));
        this->upToDate.push_back(false);
    }
}

QImage& Raw3DImage::getImage(const int& imageIdx) {
    if(!this->upToDate[imageIdx]) {
        this->convertDataToImg(imageIdx);
        this->upToDate[imageIdx] = true;
    }
    return images[imageIdx];
}

void Raw3DImage::updateMaxValue() {
    max = 0;
    for(int i = 0; i < this->data.size(); ++i) {
        uint16_t currentMax = *max_element(data[i].begin(), data[i].end());
        if(currentMax > max)
            max = currentMax;
    }
}

void Raw3DImage::convertDataToImg(const int& imageIdx) {
    this->updateMaxValue();
    for(int i = 0; i < imgSize[0]; ++i) {
        for(int j = 0; j < imgSize[1]; ++j) {
            QColor qColor;
            uint16_t value = data[imageIdx][i + j * imgSize[0]];
            if(grid->visu_map[value].r == 0.) {
                qColor = QColor(0., 0., 0.);
            } else {
                glm::vec3 color = grid->color_map[value]*glm::vec3(255., 255., 255.);
                if(color != glm::vec3(255., 255., 255.)) {
                    qColor = QColor(color.r, color.g, color.b);
                } else {
                    glm::vec3 color_0 = grid->color_0;
                    glm::vec3 color_1 = grid->color_1;
                    float blend = 0.;
                    if(max > 0)
                        blend = float(value)/float(max);
                    qColor = QColor(color_0.r * (1-blend) * 255. + color_1.r * blend * 255.,
                            color_0.g * (1-blend) * 255. + color_1.g * blend * 255.,
                            color_0.b * (1-blend) * 255. + color_1.b * blend * 255.
                            );
                }
            }
            this->images[imageIdx].setPixelColor(i, j, qColor);
        }
    }
}

/////////////////

void Image2DViewer::init() {
    this->mirrorX = false;
    this->mirrorY = false;
    this->zoomSpeed = 30;
    this->zoom = 1;
    this->imageData = QImage(10., 10., format);
    this->imageData.fill(QColor(0., 0., 0.));
    this->targetImageSize = this->imageData.size();
    this->paintedImageOrigin = QPoint(0, 0);
    this->paintedImageSize = this->imageData.size();

    this->setMouseTracking(true);
    this->activated = false;
    this->layout = new QHBoxLayout();
    this->setLayout(this->layout);
    this->display = new QLabel();
    this->layout->addWidget(this->display, 1);
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->display->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    this->minimumSize = 15;
}

void Image2DViewer::toggleShow() {
    if(this->isVisible()) {
        this->hide();
    } else {
        this->show();
    }
}

void Image2DViewer::show() {
    if(this->size().width() < minimumSize || this->size().height() < minimumSize) {
        this->activated = false;
    } else {
        this->activated = true;
        this->fitToWindow();
        this->draw();
    }
    QWidget::show();
}

void Image2DViewer::hide() {
    QWidget::hide();
    this->activated = false;
}

void Image2DViewer::clearColor() {
    this->imageData.fill(QColor(0, 0, 0));
}

void Image2DViewer::setImageSize(const QSize& targetImageSize, bool mirrorX, bool mirrorY) {
    this->mirrorX = mirrorX;
    this->mirrorY = mirrorY;
    this->targetImageSize = targetImageSize;
    this->zoomSpeed = std::max(this->targetImageSize.width(), this->targetImageSize.height())/10;

    this->paintedImageOrigin = QPoint(0, 0);
    this->paintedImageSize = targetImageSize;
    this->fitToWindow();
}

void Image2DViewer::updateImageData(const QImage& image) {
    this->imageData = image;
}

void Image2DViewer::draw() {
    if(!activated)
        return;
    QPixmap finalScreen(this->size().width(), this->size().height());
    finalScreen.fill(Qt::black);
    QPainter painter(&finalScreen);
    QRect target(paintedImageOrigin.x(), paintedImageOrigin.y(), this->paintedImageSize.width(), this->paintedImageSize.height());
    painter.drawPixmap(target, QPixmap::fromImage(this->imageData.scaled(this->paintedImageSize, Qt::IgnoreAspectRatio, Qt::FastTransformation).mirrored(mirrorX, mirrorY)));
    this->display->setPixmap(finalScreen);
}

void Image2DViewer::resizeEvent(QResizeEvent *) {
    if(this->size().width() < minimumSize || this->size().height() < minimumSize) {
        this->activated = false;
    } else {
        this->activated = true;
        this->fitToWindow();
        this->draw();
    }
}

void Image2DViewer::fitToWindow() {
    if(!activated)
        return;
    float dx = std::max(float(minimumSize), float(this->size().width()))/float(this->targetImageSize.width());
    float dy = std::max(float(minimumSize), float(this->size().height()))/float(this->targetImageSize.height());
    float df = std::min(dx, dy);
    this->paintedImageSize = QSize(std::floor(float(this->targetImageSize.width()) * df), std::floor(float(this->targetImageSize.height()) * df));
    this->paintedImageOrigin.rx() = std::max(0., std::floor(float(this->size().width() - this->paintedImageSize.width())/2.));
    this->paintedImageOrigin.ry() = std::max(0., std::floor(float(this->size().height() - this->paintedImageSize.height())/2.));
}

void Image2DViewer::mouseMoveEvent(QMouseEvent* event) {
    //std::cout << event->pos().x() << std::endl;
    if(this->inMoveMode) {
        QPoint currentPosition = event->pos();
        this->paintedImageOrigin += (currentPosition - this->movementOrigin);
        this->movementOrigin = currentPosition;
        this->draw();
        event->setAccepted(true);
    }
    QRect target(paintedImageOrigin.x(), paintedImageOrigin.y(), this->paintedImageSize.width(), this->paintedImageSize.height());
    if(target.contains(event->pos())) {
        QPoint pointInPaintedImage = event->pos() - paintedImageOrigin;
        if(this->mirrorX)
            pointInPaintedImage.rx() = (paintedImageOrigin.x() + paintedImageSize.width()) - event->pos().x();
        if(this->mirrorY)
            pointInPaintedImage.ry() = (paintedImageOrigin.y() + paintedImageSize.height()) - event->pos().y();
        glm::vec2 ptInPaintedImage(pointInPaintedImage.x(), pointInPaintedImage.y());
        //glm::vec2 targetSize = glm::vec2(dataImageSize.width(), dataImageSize.height());
        //glm::vec2 srcSize = glm::vec2(paintedImageSize.width(), paintedImageSize.height());
        //glm::ivec2 ptInImage = ptInPaintedImage * (targetSize / srcSize);
        //Q_EMIT(mouseMovedIn2DPlanarViewer(ptInImage));
    }
}

void Image2DViewer::mousePressEvent(QMouseEvent* event) {
    std::cout << "Move mode" << std::endl;
    this->inMoveMode = true;
    this->movementOrigin = event->pos();
    Q_EMIT isSelected();
}

void Image2DViewer::mouseReleaseEvent(QMouseEvent* event) {
    std::cout << "Release mode false" << std::endl;
    this->inMoveMode = false;
}

void Image2DViewer::wheelEvent(QWheelEvent *event) {
    if(event->angleDelta().y() > 0) {
        //zoom += this->zoomSpeed;
        this->paintedImageSize.rwidth() += this->zoomSpeed;
        this->paintedImageSize.rheight() += this->zoomSpeed;
    } else if(event->angleDelta().y() < 0) {
        //zoom -= this->zoomSpeed;
        this->paintedImageSize.rwidth() -= this->zoomSpeed;
        this->paintedImageSize.rheight() -= this->zoomSpeed;
    }
    this->draw();
    event->setAccepted(false);
}

/////////////////

void Image3DViewer::init(const glm::vec3& imageSize, const int& sliceIdx, const glm::vec3& side, std::vector<std::string> gridNames, std::vector<int> imgToDraw, std::vector<int> alphaValues, std::vector<std::pair<QColor, QColor>> colors, Interpolation::Method interpolationMethod, std::pair<bool, bool> mirror) {

    for(auto name : gridNames)
        if(name.empty())
            return;

    this->direction = side;

    this->imageSize = imageSize;

    this->gridNames = gridNames;
    this->imagesToDraw = imgToDraw;
    this->alphaValues = alphaValues;
    this->colors = colors;
    this->interpolationMethod = interpolationMethod;

    this->upToDate.clear();
    this->upToDate = std::vector<std::vector<bool>>(gridNames.size(), std::vector<bool>(this->imageSize.z, false));

    imgData.clear();
    imgData.reserve(gridNames.size());
    for(auto name : gridNames)
        imgData.push_back(Raw3DImage(this->imageSize, scene->grids[scene->getGridIdx(name)], imgFormat));

    this->isInitialized = true;

    this->viewer2D->setImageSize(QSize(imageSize.x, imageSize.y), mirror.first, mirror.second);
    this->setSliceIdx(sliceIdx);

}

void Image3DViewer::setSliceIdx(int newSliceIdx) {
    this->sliceIdx = newSliceIdx;
    if(this->isInitialized) {
        this->drawImages();
    }
}

void Image3DViewer::saveImagesSlices(const QString& fileName) {
    this->fillAllImagesSlices();
    this->scene->writeGreyscaleTIFFImage(fileName.toStdString(), this->imageSize, this->imgData[this->imagesToDraw[0]].data);
    //for(int sliceIdx = 0; sliceIdx < this->upToDate[0].size(); ++sliceIdx) {
    //    QString fileName = path + QString("/slice") + QString(std::to_string(sliceIdx).c_str()) + QString(".png");
    //    this->getMergedImage(sliceIdx).save(fileName);
    //}
}

void Image3DViewer::reset() {
    for(auto& line : this->upToDate)
        std::fill(line.begin(), line.end(), false);
}

void Image3DViewer::initLayout() {
    imgFormat = QImage::Format_RGB16;
    mergedImgFormat = QImage::Format_ARGB32;
    this->viewer2D = new Image2DViewer(mergedImgFormat, this);
}

void Image3DViewer::fillCurrentImages() {
    for(int i = 0; i < this->imagesToDraw.size(); ++i) {
        if(!this->upToDate[this->imagesToDraw[i]][this->sliceIdx]) {
            this->fillImage(this->imagesToDraw[i], this->sliceIdx);
            this->upToDate[this->imagesToDraw[i]][this->sliceIdx] = true;
        }
    }
}

void Image3DViewer::fillAllImagesSlices() {
    for(int sliceIdx = 0; sliceIdx < this->upToDate[0].size(); ++sliceIdx) {
        std::cout << "Fill image " << sliceIdx << std::endl;
        for(int i = 0; i < this->imagesToDraw.size(); ++i) {
            if(!this->upToDate[this->imagesToDraw[i]][sliceIdx]) {
                this->fillImage(this->imagesToDraw[i], sliceIdx);
                this->upToDate[this->imagesToDraw[i]][sliceIdx] = true;
            }
        }
    }
}

void Image3DViewer::fillImage(int imageIdx, int sliceIdx) {
    std::vector<uint16_t> data;
    auto bbox = scene->getBbox(this->gridNames[0]);
    glm::vec3 slices(-1, -1, sliceIdx);
    if(this->direction == glm::vec3(1., 0., 0.))
        std::swap(slices.x, slices.z);
    if(this->direction == glm::vec3(0., 1., 0.))
        std::swap(slices.y, slices.z);
    if(this->direction == glm::vec3(0., 0., 1.))
        std::swap(slices.z, slices.z);
    scene->getValues(this->gridNames[imageIdx], slices, bbox, this->imageSize, data, this->interpolationMethod);
    this->imgData[imageIdx].setSlice(sliceIdx, data);
}

void Image3DViewer::getColor(int idx, glm::ivec3 position, QColor& color) {
    color = this->imgData[idx].getImage(position.z).pixelColor(position.x, position.y);
}

QImage Image3DViewer::getCurrentMergedImage() {
    return this->mergeImages(this->imagesToDraw, this->sliceIdx);
}

QImage Image3DViewer::getMergedImage(int sliceIdx) {
    return this->mergeImages(this->imagesToDraw, sliceIdx);
}

QImage Image3DViewer::mergeImages(const std::vector<int>& indexes, const int& z) {
    QColor color;
    QPixmap result(this->imageSize.x, this->imageSize.y);
    //result.fill(Qt::black);
    result.fill(this->colors[indexes[0]].first);
    QPainter painter(&result);

    for(int k = 0; k < indexes.size(); ++k) {
        int idx = indexes[k];
        QImage img = this->imgData[idx].getImage(z).convertToFormat(mergedImgFormat);
        for(int i = 0; i < img.width(); ++i) {
            for(int j = 0; j < img.height(); ++j) {
                QColor color = img.pixelColor(i, j);
                if(color == Qt::black && idx > 0) {
                    color.setAlpha(0);
                } else {
                    //float value = float(color.red())/255.;
                    //color = QColor(this->colors[idx].first.red()*(1.-value)+this->colors[idx].second.red()*value,
                    //               this->colors[idx].first.green()*(1.-value)+this->colors[idx].second.green()*value,
                    //               this->colors[idx].first.blue()*(1.-value)+this->colors[idx].second.blue()*value);
                    color.setAlpha(alphaValues[idx]);
                }
                img.setPixelColor(i, j, color);
            }
        }
        painter.drawPixmap(QPoint(0, 0), QPixmap::fromImage(img));
    }

    return result.toImage().convertToFormat(mergedImgFormat);
}

void Image3DViewer::drawImages() {
    if(this->viewer2D->activated) {
        this->fillCurrentImages();
        this->viewer2D->updateImageData(this->getCurrentMergedImage());
        this->viewer2D->draw();
    } else {
        std::cout << "Viewer deactivated !!" << std::endl;
    }
}

void Image3DViewer::convertVector(glm::vec3& vec) {
    if(this->direction == glm::vec3(1., 0., 0.))
        std::swap(vec.x, vec.z);
    if(this->direction == glm::vec3(0., 1., 0.))
        std::swap(vec.y, vec.z);
    if(this->direction == glm::vec3(0., 0., 1.))
        std::swap(vec.z, vec.z);
}

void Image3DViewer::mouseMovedIn2DViewer(const glm::ivec2& positionOfMouse2D) {
    std::cout << positionOfMouse2D << std::endl;
    glm::vec3 positionOfMouse3D(positionOfMouse2D.x, positionOfMouse2D.y, sliceIdx);
    glm::vec3 convertedTargetImgSize = this->imageSize;
    //glm::vec3 convertedOriginalImgSize = this->originalImgSize;
    glm::vec3 convertedOriginalImgSize = this->imageSize;
    convertVector(convertedOriginalImgSize);
    convertVector(convertedTargetImgSize);
    convertVector(positionOfMouse3D);
    glm::vec3 fromTargetToOriginal = convertedOriginalImgSize / glm::vec3(convertedTargetImgSize);
    positionOfMouse3D *= fromTargetToOriginal;
    this->scene->grids[this->scene->getGridIdx(this->gridNames[0])]->fromImageToWorld(positionOfMouse3D);
    Q_EMIT(mouseMovedInPlanarViewer(positionOfMouse3D));
}

void Image3DViewer::connect(Scene * scene) {
    //QObject::connect(scene, &Scene::meshMoved, [this, scene](){
    //    this->reset();
    //    this->drawImages();
    //});

    QObject::connect(this->viewer2D, &Image2DViewer::isSelected, this, &Image3DViewer::isSelected);
    QObject::connect(this->viewer2D, &Image2DViewer::mouseMovedIn2DPlanarViewer, this, &Image3DViewer::mouseMovedIn2DViewer);
    //QObject::connect(this, &Image3DViewer::mouseMovedInPlanarViewer, scene, &Scene::previewPointInPlanarView);
    QObject::connect(this, &Image3DViewer::mouseMovedInPlanarViewer, scene, &Scene::pointIsClickedInPlanarViewer);
}

//////////////////////////

void PlanarViewForm::addViewer(const QString& name, const glm::vec3& side) {
    if(name.isEmpty())
        return;
    this->viewers[name] = new Image3DViewer(name, side, this->scene);
    QObject::connect(this->viewers[name], &Image3DViewer::isSelected, [=](){this->selectViewer(name);});

    this->viewersValues[name] = glm::ivec3(0, 0, 0);
    this->viewersRes[name] = {glm::vec3(1, 1, 1), glm::vec3(1, 1, 1), glm::vec3(1, 1, 1)};
    this->viewersMirror[name] = {{false, false}, {false, false}, {false, false}};

    this->selectedViewer = name;
    this->labels["SelectedViewer"]->setText(name);

    glm::vec3 imgSize = this->getBackImgDimension(scene);
    if(side.x == 1.)
        std::swap(imgSize.x, imgSize.z);
    if(side.y == 1.)
        std::swap(imgSize.y, imgSize.z);
    this->setSpinBoxesValues(imgSize);

    this->updateImageViewer();
}

void PlanarViewForm::selectViewer(const QString& name) {
    this->storeCurrentValues();
    this->selectedViewer = name;
    this->labels["SelectedViewer"]->setText(name);
    this->updateDefaultValues(name);
    this->recoverValues();
    this->updateSlice();
}

void PlanarViewForm::updateDefaultValues(const QString& name) {
    const Image3DViewer * viewer = this->viewers[name];

    this->setSpinBoxesValues(viewer->imageSize);

    this->comboBoxes["Interpolation"]->blockSignals(true);
    int idx = this->comboBoxes["Interpolation"]->findText(Interpolation::toString(viewer->interpolationMethod).c_str());
    this->comboBoxes["Interpolation"]->setCurrentIndex(idx);
    this->comboBoxes["Interpolation"]->blockSignals(false);

    this->objectChoosers["From"]->blockSignals(true);
    idx = 0;
    if(viewer->gridNames.size() > 0) {
        this->spinBoxes["AlphaBack"]->blockSignals(true);
        this->spinBoxes["AlphaBack"]->setValue(viewer->alphaValues[0]);
        this->spinBoxes["AlphaBack"]->blockSignals(false);

        idx = this->objectChoosers["From"]->findText(viewer->gridNames[0].c_str());
    }
    this->objectChoosers["From"]->setCurrentIndex(idx);
    this->objectChoosers["From"]->blockSignals(false);

    this->objectChoosers["To"]->blockSignals(true);
    idx = 0;
    if(viewer->gridNames.size() > 1) {
        this->spinBoxes["AlphaFront"]->blockSignals(true);
        this->spinBoxes["AlphaFront"]->setValue(viewer->alphaValues[1]);
        this->spinBoxes["AlphaFront"]->blockSignals(false);

        idx = this->objectChoosers["To"]->findText(viewer->gridNames[1].c_str());
    }
    this->objectChoosers["To"]->setCurrentIndex(idx);
    this->objectChoosers["To"]->blockSignals(false);

    this->checkBoxes["UseBack"]->blockSignals(true);
    this->checkBoxes["UseFront"]->blockSignals(true);

    std::vector imgsToDraw = viewer->imagesToDraw;
    this->checkBoxes["UseBack"]->setChecked(std::find(imgsToDraw.begin(), imgsToDraw.end(), 0) != imgsToDraw.end());
    this->checkBoxes["UseFront"]->setChecked(std::find(imgsToDraw.begin(), imgsToDraw.end(), 1) != imgsToDraw.end());

    this->checkBoxes["UseBack"]->blockSignals(false);
    this->checkBoxes["UseFront"]->blockSignals(false);

    this->sliders["SliderX"]->blockSignals(true);
    this->sliders["SliderX"]->setValue(viewer->sliceIdx);
    this->sliders["SliderX"]->setMaximum(viewer->imageSize.z-1);
    this->sliders["SliderX"]->blockSignals(false);

    this->blockSignalsInGroup("GroupSide", true);
    if(viewer->direction == glm::vec3(1., 0., 0.)) {
        this->buttons["SideX"]->setChecked(true);
        this->buttons["SideY"]->setChecked(false);
        this->buttons["SideZ"]->setChecked(false);
    }
    if(viewer->direction == glm::vec3(0., 1., 0.)) {
        this->buttons["SideX"]->setChecked(false);
        this->buttons["SideY"]->setChecked(true);
        this->buttons["SideZ"]->setChecked(false);
    }
    if(viewer->direction == glm::vec3(0., 0., 1.)) {
        this->buttons["SideX"]->setChecked(false);
        this->buttons["SideY"]->setChecked(false);
        this->buttons["SideZ"]->setChecked(true);
    }
    this->blockSignalsInGroup("GroupSide", false);

}

void PlanarViewForm::init(Scene * scene) {

    this->add(WidgetType::H_GROUP, "GroupHeader");
    this->addAllNextWidgetsToGroup("GroupHeader");

    this->add(WidgetType::LABEL, "SelectedViewer", "NONE");
    //this->add(WidgetType::BUTTON, "Rotate");
    //QPixmap pixmap(QString("../resources/rotate.svg"));
    //QIcon ButtonIcon(pixmap);
    //this->buttons["Rotate"]->setIcon(ButtonIcon);
    //this->buttons["Rotate"]->setText("");
    //this->buttons["Rotate"]->setIconSize(pixmap.rect().size());

    this->add(WidgetType::TIFF_SAVE, "Save");
    this->add(WidgetType::TIFF_SAVE, "SaveCur");

    this->add(WidgetType::BUTTON_CHECKABLE, "Link");
    QPixmap pixmap2(QString("../resources/link.svg"));
    QIcon ButtonIcon2(pixmap2);
    this->buttons["Link"]->setIcon(ButtonIcon2);
    this->buttons["Link"]->setText("");
    this->buttons["Link"]->setIconSize(pixmap2.rect().size());

    this->addAllNextWidgetsToDefaultGroup();

    this->addWithLabel(WidgetType::SLIDER, "SliderX", "X");
    this->sliders["SliderX"]->setMinimum(0);
    this->sliders["SliderX"]->setMaximum(0);
    this->labels["SliderX"]->setFixedWidth(50);

    this->addWithLabel(WidgetType::H_GROUP, "GroupBack", "Back");
    this->addAllNextWidgetsToGroup("GroupBack");
    this->groups["GroupBack"]->setAlignment(Qt::AlignHCenter);

    this->add(WidgetType::GRID_CHOOSE, "From", "Back");
    this->setObjectTypeToChoose("From", ObjectToChoose::GRID);
    this->objectChoosers["From"]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->add(WidgetType::CHECK_BOX, "UseBack");
    this->add(WidgetType::SPIN_BOX, "AlphaBack");
    this->spinBoxes["AlphaBack"]->setSingleStep(20);

    this->addAllNextWidgetsToDefaultGroup();

    this->addWithLabel(WidgetType::H_GROUP, "GroupFront", "Front");
    this->addAllNextWidgetsToGroup("GroupFront");
    this->groups["GroupFront"]->setAlignment(Qt::AlignHCenter);

    this->add(WidgetType::GRID_CHOOSE, "To", "Front");
    this->setObjectTypeToChoose("To", ObjectToChoose::GRID);
    this->objectChoosers["To"]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->add(WidgetType::CHECK_BOX, "UseFront");
    this->add(WidgetType::SPIN_BOX, "AlphaFront");
    this->spinBoxes["AlphaFront"]->setSingleStep(20);

    this->addAllNextWidgetsToDefaultGroup();

    this->addWithLabel(WidgetType::COMBO_BOX, "Interpolation", "Interpolation");
    this->setComboChoices("Interpolation", Interpolation::toStringList());

    this->addWithLabel(WidgetType::H_GROUP, "GroupResolution", "Resolution");
    this->addAllNextWidgetsToGroup("GroupResolution");

    this->add(WidgetType::SPIN_BOX, "X");
    this->add(WidgetType::SPIN_BOX, "Y");
    this->add(WidgetType::SPIN_BOX, "Z");
    this->add(WidgetType::BUTTON, "Auto");

    this->addAllNextWidgetsToDefaultGroup();

    this->addWithLabel(WidgetType::H_GROUP, "GroupSide", "Side");
    this->addAllNextWidgetsToGroup("GroupSide");

    this->add(WidgetType::BUTTON_CHECKABLE_AUTOEXCLUSIVE, "SideX", "X");
    this->add(WidgetType::BUTTON_CHECKABLE_AUTOEXCLUSIVE, "SideY", "Y");
    this->add(WidgetType::BUTTON_CHECKABLE_AUTOEXCLUSIVE, "SideZ", "Z");

    this->addAllNextWidgetsToDefaultGroup();

    this->addWithLabel(WidgetType::H_GROUP, "GroupMirror", "Mirror");
    this->addAllNextWidgetsToGroup("GroupMirror");

    this->add(WidgetType::BUTTON_CHECKABLE, "MirrorX", "X");
    this->add(WidgetType::BUTTON_CHECKABLE, "MirrorY", "Y");

    this->addAllNextWidgetsToDefaultGroup();

    /****/

    this->checkBoxes["UseBack"]->setChecked(true);
    this->checkBoxes["UseFront"]->setChecked(true);

    this->spinBoxes["AlphaBack"]->setMinimum(0);
    this->spinBoxes["AlphaBack"]->setMaximum(255);
    this->spinBoxes["AlphaBack"]->setValue(255);
    this->spinBoxes["AlphaFront"]->setMinimum(0);
    this->spinBoxes["AlphaFront"]->setMaximum(255);
    this->spinBoxes["AlphaFront"]->setValue(255);

    this->setDisabled(true);
    /****/

    //this->addViewer("ViewZ");
}

void PlanarViewForm::initViewer(const QString& name) {
    this->setDisabled(false);
    this->show();
    this->selectViewer(name);
    this->checkBoxes["UseBack"]->blockSignals(true);
    this->checkBoxes["UseFront"]->blockSignals(true);
    this->checkBoxes["UseBack"]->setChecked(true);
    this->checkBoxes["UseFront"]->setChecked(true);
    this->checkBoxes["UseBack"]->blockSignals(false);
    this->checkBoxes["UseFront"]->blockSignals(false);
    this->viewers[name]->viewer2D->activated = true;
    this->buttons["Link"]->setChecked(false);
    this->buttons["SideX"]->setChecked(false);
    this->buttons["SideY"]->setChecked(false);
    this->buttons["SideZ"]->setChecked(false);

    this->buttons["Link"]->click();
    if(this->getSide().x == 1.)
        this->buttons["SideX"]->click();
    if(this->getSide().y == 1.)
        this->buttons["SideY"]->click();
    if(this->getSide().z == 1.)
        this->buttons["SideZ"]->click();
}

glm::vec3 PlanarViewForm::getBackImgDimension(Scene * scene) {
    glm::vec3 defaultValue = glm::vec3(1., 1., 1.);
    std::string name = this->getFromGridName();
    if(name == "")
        return defaultValue;
    //return scene->grids[scene->getGridIdx(name)]->grid->getResolution();
    return scene->grids[scene->getGridIdx(name)]->getDimensions();
}

void PlanarViewForm::backImageChanged(Scene * scene) {
    glm::vec3 imgSize = this->getBackImgDimension(scene);
    if(this->getSide().x == 1.)
        std::swap(imgSize.x, imgSize.z);
    if(this->getSide().y == 1.)
        std::swap(imgSize.y, imgSize.z);
    this->setSpinBoxesValues(imgSize);
}

glm::ivec3 PlanarViewForm::autoComputeBestSize(Scene * scene) {
    if(this->getFromGridName().empty() || this->getToGridName().empty())
        return glm::ivec3(1, 1, 1);
    glm::vec3 gridResolution = scene->getGridImgSize(this->getFromGridName());
    glm::vec3 voxelSize = scene->getGridVoxelSize(this->getFromGridName());
    float maxSize = std::min(voxelSize.x, std::min(voxelSize.y, voxelSize.z));
    glm::ivec3 finalSize(0., 0., 0.);
    for(int i = 0; i < 3; ++i) {
        float ratio = voxelSize[i]/maxSize;
        finalSize[i] = std::floor(ratio*gridResolution[i]);
    }
    std::cout << "Auto compute size: " << finalSize << std::endl;
    return finalSize;
}

glm::vec3 PlanarViewForm::getSide() {
    if(this->noViewerSelected())
        return glm::vec3(0., 0., 1.);
    return this->viewers[this->selectedViewer]->direction;
}

void PlanarViewForm::setAutoImageResolution() {
    glm::vec3 dim = this->autoComputeBestSize(this->scene);
    convertVector(dim);
    this->setSpinBoxesValues(dim);
    this->updateImageViewer();
}

void PlanarViewForm::setSpinBoxesValues(const glm::vec3& values) {
    this->spinBoxes["X"]->blockSignals(true);
    this->spinBoxes["Y"]->blockSignals(true);
    this->spinBoxes["Z"]->blockSignals(true);
    this->spinBoxes["X"]->setMinimum(1);
    this->spinBoxes["Y"]->setMinimum(1);
    this->spinBoxes["Z"]->setMinimum(1);
    this->spinBoxes["X"]->setValue(values.x);
    this->spinBoxes["Y"]->setValue(values.y);
    this->spinBoxes["Z"]->setValue(values.z);
    this->spinBoxes["X"]->blockSignals(false);
    this->spinBoxes["Y"]->blockSignals(false);
    this->spinBoxes["Z"]->blockSignals(false);
}

bool PlanarViewForm::noViewerSelected() {
    return !this->viewers[this->selectedViewer];
}

void PlanarViewForm::convertVector(glm::vec3& vec) {
    if(this->getSide() == glm::vec3(1., 0., 0.))
        std::swap(vec.x, vec.z);
    if(this->getSide() == glm::vec3(0., 1., 0.))
        std::swap(vec.y, vec.z);
    if(this->getSide() == glm::vec3(0., 0., 1.))
        std::swap(vec.z, vec.z);
}

void PlanarViewForm::updateImageViewer() {
    if(this->noViewerSelected() || this->isHidden())
        return;
    this->sliders["SliderX"]->setMinimum(0);
    this->sliders["SliderX"]->setMaximum(this->getImgDimension().z-1);
    glm::vec3 finalImageSize = autoComputeBestSize(scene);
    convertVector(finalImageSize);
    glm::vec3 originalImgDimension = this->getBackImgDimension(scene);
    convertVector(originalImgDimension);

    glm::vec3 color0_0 = glm::vec3(1., .0, .0);
    glm::vec3 color1_0 = glm::vec3(.0, .0, 1.);

    if(this->scene->grids.size() > 0) {
        color0_0 = this->scene->grids[0]->color_0;
        color1_0 = this->scene->grids[0]->color_1;
    }

    glm::vec3 color0_1 = glm::vec3(1., .0, .0);
    glm::vec3 color1_1 = glm::vec3(.0, .0, 1.);

    if(this->scene->grids.size() > 1) {
        color0_1 = this->scene->grids[1]->color_0;
        color1_1 = this->scene->grids[1]->color_1;
    }

    this->viewers[this->selectedViewer]->init(
            //this->getImgDimension(),
            //finalImageSize,
            originalImgDimension,
            this->sliders["SliderX"]->value(),
            this->getSide(),
            {this->getFromGridName(), this->getToGridName()},
            this->getImagesToDraw(),
            {this->spinBoxes["AlphaBack"]->value(), this->spinBoxes["AlphaFront"]->value()},
            {std::make_pair(
                    QColor(255.*color0_1.x, 255.*color0_1.y, 255.*color0_1.z),
                    QColor(255.*color1_1.x, 255.*color1_1.y, 255.*color1_1.z)),
            std::make_pair(
                    QColor(255.*color0_0.x, 255.*color0_0.y, 255.*color0_0.z),
                    QColor(255.*color1_0.x, 255.*color1_0.y, 255.*color1_0.z))
            },
            this->getInterpolationMethod(),
            {this->buttons["MirrorX"]->isChecked(), this->buttons["MirrorY"]->isChecked()});
}

void PlanarViewForm::update(Scene * scene) {
    Form::update(scene);
}

void PlanarViewForm::show() {
    Form::show();
}

glm::ivec3 PlanarViewForm::getImgDimension() {
    glm::ivec3 value = glm::ivec3(this->spinBoxes["X"]->value(), this->spinBoxes["Y"]->value(), this->spinBoxes["Z"]->value());
    if(value.x > 100000 || value.y > 100000 || value.z > 100000)
        value = glm::ivec3(10, 10, 10);
    return value;
}

Interpolation::Method PlanarViewForm::getInterpolationMethod() {
    QString method(this->comboBoxes["Interpolation"]->currentText());
    return Interpolation::fromString(method.toStdString());
}

std::vector<int> PlanarViewForm::getImagesToDraw() {
    std::vector<int> imagesToDraw;
    if(this->checkBoxes["UseBack"]->isChecked())
        imagesToDraw.push_back(0);
    if(this->checkBoxes["UseFront"]->isChecked())
        imagesToDraw.push_back(1);
    return imagesToDraw;
}

std::string PlanarViewForm::getFromGridName() {
    return this->objectChoosers["From"]->currentText().toStdString();
}

std::string PlanarViewForm::getToGridName() {
    return this->objectChoosers["To"]->currentText().toStdString();
}

void PlanarViewForm::storeCurrentValues() {
    if(this->noViewerSelected())
        return;
    glm::vec3 side = this->getSide();
    int value = this->sliders["SliderX"]->value();
    glm::vec3 res = this->getImgDimension();
    std::pair<bool, bool> mirror = {this->buttons["MirrorX"]->isChecked(), this->buttons["MirrorY"]->isChecked()};
    if(side.x > 0) {
        this->viewersValues[this->selectedViewer].x = value;
        this->viewersRes[this->selectedViewer][0] = res;
        this->viewersMirror[this->selectedViewer][0] = mirror;
    }

    if(side.y > 0) {
        this->viewersValues[this->selectedViewer].y = value;
        this->viewersRes[this->selectedViewer][1] = res;
        this->viewersMirror[this->selectedViewer][1] = mirror;
    }

    if(side.z > 0) {
        this->viewersValues[this->selectedViewer].z = value;
        this->viewersRes[this->selectedViewer][2] = res;
        this->viewersMirror[this->selectedViewer][2] = mirror;
    }
    std::cout << "Store value: " << value << std::endl;
}

void PlanarViewForm::recoverValues() {
    if(this->noViewerSelected())
        return;
    glm::vec3 side = this->getSide();
    int value = 0;
    glm::vec3 res = glm::vec3(1, 1, 1);
    std::pair<bool, bool> mirror = {false, false};
    if(side.x > 0) {
        value = this->viewersValues[this->selectedViewer].x;
        res = this->viewersRes[this->selectedViewer][0];
        mirror = this->viewersMirror[this->selectedViewer][0];
    }

    if(side.y > 0) {
        value = this->viewersValues[this->selectedViewer].y;
        res = this->viewersRes[this->selectedViewer][1];
        mirror = this->viewersMirror[this->selectedViewer][1];
    }

    if(side.z > 0) {
        value = this->viewersValues[this->selectedViewer].z;
        res = this->viewersRes[this->selectedViewer][2];
        mirror = this->viewersMirror[this->selectedViewer][2];
    }
    if(res.x > 1 && res.y > 1 && res.z > 1)
        this->setSpinBoxesValues(res);
    this->sliders["SliderX"]->blockSignals(true);
    this->sliders["SliderX"]->setMinimum(0);
    this->sliders["SliderX"]->setMaximum(this->getImgDimension().z-1);
    this->sliders["SliderX"]->setValue(value);
    this->sliders["SliderX"]->blockSignals(false);
    this->buttons["MirrorX"]->blockSignals(true);
    this->buttons["MirrorX"]->setChecked(mirror.first);
    this->buttons["MirrorX"]->blockSignals(false);
    this->buttons["MirrorY"]->blockSignals(true);
    this->buttons["MirrorY"]->setChecked(mirror.second);
    this->buttons["MirrorY"]->blockSignals(false);
}

void PlanarViewForm::updateSlice() {
    this->viewers[this->selectedViewer]->setSliceIdx(this->sliders["SliderX"]->value());
    if(this->buttons["Link"]->isChecked()) {
        if(this->viewers[this->selectedViewer]->direction == glm::vec3(1., 0., 0.)) {
            this->scene->slotSetNormalizedPlaneDisplacement(Scene::CuttingPlaneDirection::X, float(this->sliders["SliderX"]->value())/float(this->sliders["SliderX"]->maximum()));
            this->scene->slotSetNormalizedPlaneDisplacement(Scene::CuttingPlaneDirection::Y, 0.);
            this->scene->slotSetNormalizedPlaneDisplacement(Scene::CuttingPlaneDirection::Z, 0.);
        }
        if(this->viewers[this->selectedViewer]->direction == glm::vec3(0., 1., 0.)) {
            this->scene->slotSetNormalizedPlaneDisplacement(Scene::CuttingPlaneDirection::Y, float(this->sliders["SliderX"]->value())/float(this->sliders["SliderX"]->maximum()));
            this->scene->slotSetNormalizedPlaneDisplacement(Scene::CuttingPlaneDirection::Z, 0.);
            this->scene->slotSetNormalizedPlaneDisplacement(Scene::CuttingPlaneDirection::X, 0.);
        }
        if(this->viewers[this->selectedViewer]->direction == glm::vec3(0., 0., 1.)) {
            this->scene->slotSetNormalizedPlaneDisplacement(Scene::CuttingPlaneDirection::Z, float(this->sliders["SliderX"]->value())/float(this->sliders["SliderX"]->maximum()));
            this->scene->slotSetNormalizedPlaneDisplacement(Scene::CuttingPlaneDirection::X, 0.);
            this->scene->slotSetNormalizedPlaneDisplacement(Scene::CuttingPlaneDirection::Y, 0.);
        }
        //this->scene->updatePlaneControlWidget();
    }
    this->labels["SliderX"]->setText(std::to_string(this->sliders["SliderX"]->value()).c_str());
}

void PlanarViewForm::connect(Scene * scene) {
    QObject::connect(this, &Form::widgetModified, [this, scene](const QString &id){
            if(this->noViewerSelected())
            return;
            if(id == "From")
            this->backImageChanged(scene);

            if(id == "Auto")
            this->setAutoImageResolution();

            if(id == "X" || id == "Y" || id == "Z" || id == "Interpolation" || id == "UseBack" || id == "UseFront" || id == "From" || id == "To" || id == "AlphaBack" || id == "AlphaFront" || "MirrorX" || "MirrorY")
            this->updateImageViewer();

            if(id == "SideX" || id == "SideY" || id == "SideZ") {
            this->storeCurrentValues();
            if(id == "SideX")
            this->viewers[this->selectedViewer]->direction = glm::vec3(1., 0., 0.);
            if(id == "SideY")
            this->viewers[this->selectedViewer]->direction = glm::vec3(0., 1., 0.);
            if(id == "SideZ")
            this->viewers[this->selectedViewer]->direction = glm::vec3(0., 0., 1.);
            this->backImageChanged(scene);
            this->recoverValues();
            this->updateImageViewer();
            }

            if(id == "SliderX" || id == "SideX" || id == "SideY" || id == "SideZ" || id == "Link") {
                this->updateSlice();
            }
    });

    QObject::connect(scene, &Scene::meshMoved, [this, scene](){
            this->updateImageViewer();
            //this->reset();
            //this->drawImages();
            });

    QObject::connect(this->fileChoosers["Save"], &FileChooser::fileSelected, [this](){
            //this->viewers[this->selectedViewer]->saveImagesSlices(this->fileChoosers["Save"]->filename);
            //this->scene->writeMapping(this->fileChoosers["Save"]->filename.toStdString(), this->getFromGridName(), this->getToGridName());
            //this->scene->sampleGridMapping(this->fileChoosers["Save"]->filename.toStdString(), this->getFromGridName(), this->getToGridName(), this->getImgDimension(), this->getInterpolationMethod());
            //this->scene->sampleGridMapping(this->fileChoosers["Save"]->filename.toStdString(), this->getFromGridName(), this->getToGridName(), this->getImgDimension(), this->getInterpolationMethod());
            this->scene->writeDeformedImage(this->fileChoosers["Save"]->filename.toStdString(), this->getFromGridName(), false, ResolutionMode::SAMPLER_RESOLUTION);
            });

    QObject::connect(this->fileChoosers["SaveCur"], &FileChooser::fileSelected, [this](){
            this->viewers[this->selectedViewer]->saveImagesSlices(this->fileChoosers["SaveCur"]->filename);
            });

    QObject::connect(scene, &Scene::colorChanged, [this, scene](){
            if(this->noViewerSelected())
            return;
            updateImageViewer();
            });
}
