#ifndef GL_MANIPULATOR_HPP_
#define GL_MANIPULATOR_HPP_

#include "../../third_party/primitive/Sphere.h"
#include "../../qt/viewers/include/viewer_structs.hpp"
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
			MeshManipulator(SceneGL* sceneGL, BaseMesh * base, const std::vector<glm::vec3>& positions, float manipulatorRadius = 50.f);

			void draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat);

			void setProgram(GLuint program) { this->program = program; };
			GLuint getProgram() { return this->program; };

			void setVao(GLuint vao) { this->vao = vao; };
			GLuint getVao() { return this->vao; };

			void setVboVertices(GLuint vboVertices) { this->vboVertices = vboVertices; };
			GLuint getVboVertices() { return this->vboVertices; };

			void setVboIndices(GLuint vboIndices) { this->vboIndices = vboIndices; };
			GLuint getVboIndices() { return this->vboIndices; };

			void setTex(GLuint tex) { this->tex = tex; };
			GLuint getTex() { return this->tex; };

			bool isWireframeDisplayed() { return this->displayWireframe; }

            void toggleActivation();

			UITool::MeshManipulator * meshManipulator;	 // TODO: shared pointer

        public slots:
			void prepare();

            void setRadius(float radius);
            void createNewMeshManipulator(BaseMesh * mesh, Scene * scene, int type);
			void toggleDisplayWireframe() { this->displayWireframe = ! this->displayWireframe; }

		private:

			float manipulatorRadius;
			Sphere manipulatorMesh;

			SceneGL * sceneGL;

			GLuint program;
			GLuint vao;
			GLuint vboVertices;
			GLuint vboIndices;
			GLuint tex;
			GLuint visible;

			TextureUpload texParams;
			TextureUpload texParamsVisible;

			bool displayWireframe;
		};

	}	 // namespace GL

}	 // namespace UITool

#endif
