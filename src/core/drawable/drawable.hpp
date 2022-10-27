#ifndef GL_GRAPH_HPP_
#define GL_GRAPH_HPP_

#include "../../qt/legacy/viewer_structs.hpp"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_3_2_Compatibility>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLFunctions_4_0_Core>

class SceneGL;
class GraphMesh;

namespace UITool {
	namespace GL {

        // Used by increment/decrementSize function to easily designate which primitive to draw bigger or smaller
        enum DrawingPrimitive {
            SPHERE,
            LINE,
            GUIZMO
        };

        class DrawableUI : public QObject {
            Q_OBJECT;
		public:
            DrawableUI();
            //void draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat, const glm::vec3& planeDisplacement);

        public slots:
            // Connected to zoom function in scene
            virtual void zoom(float newSceneRadius);

            void incrementSize(const UITool::GL::DrawingPrimitive& object);
            void decrementSize(const UITool::GL::DrawingPrimitive& object);

            void setSize(const UITool::GL::DrawingPrimitive& object, float size);

            float getGuizmoRadius() { return guizmoRadius; };

        protected:
            // Define the sphere's radius as a ratio of the scene size
            // Used to adapt the radius while zooming, and keep it at same size on the screen
            float sphereRatio;
            float sphereRadius;

            float linesRatio;
            float linesRadius;

            float guizmoRatio;
            float guizmoRadius;
        };
	}	 // namespace GL

}	 // namespace UITool

#endif
