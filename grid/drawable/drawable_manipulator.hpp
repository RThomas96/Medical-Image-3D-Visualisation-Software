#ifndef GL_MANIPULATOR_HPP_
#define GL_MANIPULATOR_HPP_

#include "../../third_party/primitive/Sphere.h"
#include "../../qt/legacy/viewer_structs.hpp"
#include "../ui/mesh_manipulator.hpp"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLFunctions_4_0_Core>

class SceneGL;

namespace UITool {
    //class MeshManipulator;

	/// @defgroup gl GL
	/// @brief All classes that interact with OpenGL. Allow a separation between backend and frontend.
	namespace GL {

		class MeshManipulator : QObject {
            Q_OBJECT;
		public:
			MeshManipulator(SceneGL* sceneGL, BaseMesh * base, const std::vector<glm::vec3>& positions);

            void draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat, const glm::vec3& planeDisplacement);

			bool isWireframeDisplayed() { return this->displayWireframe; }

            void toggleActivation();

            glm::vec3 lightPosition;

            bool needPreview = false;
            glm::vec3 previewPosition;

        public slots:
			void prepare();

            void createNewMeshManipulator(BaseMesh * mesh, Scene * scene, MeshManipulatorType type);
			void toggleDisplayWireframe() { this->displayWireframe = ! this->displayWireframe; }
            void toggleDisplayWireframe(bool value) { this->displayWireframe = value; }
            void updateManipulatorRadius(float sceneRadius);

        public:
            UITool::MeshManipulatorType meshManipulatorType;
			UITool::MeshManipulator * meshManipulator;	 // TODO: shared pointer

            // TODO: do not belong here
            std::vector<std::vector<glm::vec3>> persistantRegistrationToolPreviousPoints;
            std::vector<std::pair<int, std::pair<int, glm::vec3>>> persistantRegistrationToolSelectedPoints;
            std::vector<int> persistantRegistrationToolSessions;

			float planeViewRadius;
            float manipulatorRatio;
            float kidRatio;
			float manipulatorRadius;
			float kidRadius;
			Sphere manipulatorMesh;

			SceneGL * sceneGL;

			GLuint program;
			GLuint vao;
			GLuint vboVertices;
			GLuint vboNormals;
			GLuint vboIndices;
			GLuint tex;
			GLuint visible;
			GLuint state;

			TextureUpload texParams;
			TextureUpload texParamsVisible;
			TextureUpload texParamsState;

			bool displayWireframe;
		};

	}	 // namespace GL

}	 // namespace UITool

#endif
