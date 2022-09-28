#ifndef QT_IMAGE3D_VIEWER
#define QT_IMAGE3D_VIEWER

#include<glm/glm.hpp>
#include<QImage>
#include<QLabel>
#include<QHBoxLayout>

#include "../qt/legacy/viewer_structs.hpp"
#include "UI/form.hpp"

class Scene;

class Raw3DImage {

public:
    int max;

    QImage::Format format;
    glm::ivec3 imgSize;
    std::vector<std::vector<uint16_t>> data;
    GridGLView::Ptr grid;

private:
    std::vector<bool> upToDate;
    std::vector<QImage> images;

public:
    Raw3DImage(const glm::ivec3 imgSize, GridGLView::Ptr grid, QImage::Format format);
    void setSlice(const int& idx, const std::vector<uint16_t>& data);
    void setImage(const std::vector<std::vector<uint16_t>>& data);
    QImage& getImage(const int& imageIdx);

private:
    void updateMaxValue();
    void convertDataToImg(const int& imageIdx);
};

class Image2DViewer : public QWidget {
    Q_OBJECT;

public:
    Scene * scene;
    bool activated;

    // Interactions variables
    bool inMoveMode;
    QPoint movementOrigin;
    int zoomSpeed;
    float zoom;
    int minimumSize;
    bool mirrorX;
    bool mirrorY;

    // Data
    QImage imageData;// Image data to be painted
    QSize targetImageSize;// Optimal image size according to voxel size, etc

    QPoint paintedImageOrigin;
    QSize paintedImageSize;

    //Display variables
    QLabel * display;
    QHBoxLayout * layout;
    QImage::Format format;

    Image2DViewer(QImage::Format format, QWidget *parent = nullptr): QWidget(parent), format(format){init();}

    void init();
    void toggleShow();
    void show();
    void hide();
    void clearColor();
    void setImageSize(const QSize& targetImageSize, bool mirrorX, bool mirrorY);
    void updateImageData(const QImage& image);
    void draw();
    void resizeEvent(QResizeEvent *);
    void fitToWindow();
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent *event);

signals:
    void isSelected();
    void mouseMovedIn2DPlanarViewer(const glm::ivec2& positionOfMouse2D);// Used for preview
};

class Image3DViewer : public QWidget {
    Q_OBJECT

public:

    QImage::Format imgFormat;
    QImage::Format mergedImgFormat;
    QString name;

    bool isInitialized;

    glm::vec3 direction;

    glm::ivec3 imageSize;
    std::vector<std::string> gridNames;
    std::vector<int> imagesToDraw;
    std::vector<int> alphaValues;
    std::vector<std::pair<QColor, QColor>> colors;
    Interpolation::Method interpolationMethod;
    int sliceIdx;

    Image2DViewer * viewer2D;

    std::vector<std::vector<bool>> upToDate;
    std::vector<Raw3DImage> imgData;

    Scene * scene;

    Image3DViewer(const QString& name, const glm::vec3& side, Scene * scene, QWidget * parent = nullptr): QWidget(parent), name(name), direction(side), scene(scene), isInitialized(false), viewer2D(nullptr), imageSize(glm::vec3(1., 1., 1.)) {initLayout(); connect(scene);}

    void init(const glm::vec3& imageSize, const int& sliceIdx, const glm::vec3& side, std::vector<std::string> gridNames, std::vector<int> imgToDraw, std::vector<int> alphaValues, std::vector<std::pair<QColor, QColor>> colors, Interpolation::Method interpolationMethod, std::pair<bool, bool> mirror);
    void setSliceIdx(int newSliceIdx);
    void saveImagesSlices(const QString& fileName);

private:
    void reset();
    void initLayout();
    void fillCurrentImages();
    void fillAllImagesSlices();
    void fillImage(int imageIdx, int sliceIdx);
    void getColor(int idx, glm::ivec3 position, QColor& color);
    QImage getCurrentMergedImage();
    QImage getMergedImage(int sliceIdx);
    QImage mergeImages(const std::vector<int>& indexes, const int& z);
    void drawImages();
    void convertVector(glm::vec3& vec);
    void mouseMovedIn2DViewer(const glm::ivec2& positionOfMouse2D);
    void connect(Scene * scene);

signals:
    void isSelected();
    void mouseMovedInPlanarViewer(const glm::vec3& positionOfMouse3D);
};

class PlanarViewForm : public Form {
    Q_OBJECT

public:
    Scene * scene;

    //Image3DViewer * imageViewer;

    std::map<QString, std::vector<std::pair<bool, bool>>> viewersMirror;   // Store viewers res    for each side for easier usage
    std::map<QString, std::vector<glm::vec3>> viewersRes;   // Store viewers res    for each side for easier usage
    std::map<QString, glm::ivec3> viewersValues;// Store viewers values for each side for easier usage
    std::map<QString, Image3DViewer*> viewers;
    QString selectedViewer;
    PlanarViewForm(Scene * scene, QWidget *parent = nullptr):Form(parent), scene(scene){init(scene);connect(scene);}

public slots:

    void addViewer(const QString& name, const glm::vec3& side = glm::vec3(0., 0., 1.));
    void selectViewer(const QString& name);
    void updateDefaultValues(const QString& name);
    void init(Scene * scene);
    void initViewer(const QString& name);
    glm::vec3 getBackImgDimension(Scene * scene);
    void backImageChanged(Scene * scene);
    glm::ivec3 autoComputeBestSize(Scene * scene);
    glm::vec3 getSide();
    void setAutoImageResolution();
    void setSpinBoxesValues(const glm::vec3& values);
    bool noViewerSelected();
    void convertVector(glm::vec3& vec);
    void updateImageViewer();
    void update(Scene * scene);
    void show();
    glm::ivec3 getImgDimension();
    Interpolation::Method getInterpolationMethod();
    std::vector<int> getImagesToDraw();
    std::string getFromGridName();
    std::string getToGridName();
    void storeCurrentValues();
    void recoverValues();
    void updateSlice();
    void connect(Scene * scene);
};

class PlanarViewer2D : public PlanarViewForm {
    Q_OBJECT

public:
    bool initialized;
    PlanarViewer2D(Scene * scene, QWidget *parent = nullptr):PlanarViewForm(scene, parent){
        this->initialized = false;
    }

    void initialize(Scene * scene) {
        this->addViewer("View_1", glm::vec3(1., 0., 0.));
        this->addViewer("View_2", glm::vec3(0., 1., 0.));
        this->addViewer("View_3", glm::vec3(0., 0., 1.));
        this->hide();
        this->initialized = true;
    }
};

#endif
