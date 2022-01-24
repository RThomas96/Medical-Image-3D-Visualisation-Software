#ifndef GL_MANIPULATOR_HPP_
#define GL_MANIPULATOR_HPP_

#include "../../third_party/primitive/Sphere.h"
#include "../../viewer/include/viewer_structs.hpp"
#include "../../viewer/include/scene.hpp"
#include "manipulator.hpp"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLFunctions_4_0_Core>

class SceneGL;

namespace UITool {

	/// @defgroup gl GL
	/// @brief All classes that interact with OpenGL. Allow a separation between backend and frontend.
	namespace GL {

		class MeshManipulator {
		public:
			MeshManipulator(SceneGL* scene, const std::vector<glm::vec3>& positions, float manipulatorRadius = 50.f) :
				manipulatorRadius(manipulatorRadius), manipulatorMesh(Sphere(manipulatorRadius)), sceneGL(scene), meshManipulator(new UITool::DirectManipulator(positions)) {
				this->program	       = 0;
				this->vao		       = 0;
				this->vboVertices      = 0;
				this->vboIndices       = 0;
				this->tex		       = 0;
				this->displayWireframe = false;
			}

			void prepare();
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

            void setRadius(float radius);

			void toggleDisplayWireframe() { this->displayWireframe = ! this->displayWireframe; }

			bool isWireframeDisplayed() { return this->displayWireframe; }

            void addManipulator(const glm::vec3& position);

            void toggleActivation();

            void createNewMeshManipulator(const std::vector<glm::vec3>& positions, int type);

			UITool::MeshManipulator * meshManipulator;	 // TODO: shared pointer

            void removeLastManipulator() ;
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
