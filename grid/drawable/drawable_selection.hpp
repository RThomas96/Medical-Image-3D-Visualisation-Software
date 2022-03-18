#ifndef GL_SELECTION_HPP_
#define GL_SELECTION_HPP_

#include <glm/glm.hpp>
#include "glm/gtx/string_cast.hpp"

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

		class Selection : QObject {
            Q_OBJECT;
		public:
			Selection(SceneGL* sceneGL, const glm::vec3& p1, const glm::vec3& p2);

			void draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat);

			void setProgram(GLuint program) { this->program = program; };
			GLuint getProgram() { return this->program; };

			void setVao(GLuint vao) { this->vao = vao; };
			GLuint getVao() { return this->vao; };

			void setVboVertices(GLuint vboVertices) { this->vboVertices = vboVertices; };
			GLuint getVboVertices() { return this->vboVertices; };

			void setVboIndices(GLuint vboIndices) { this->vboIndices = vboIndices; };
			GLuint getVboIndices() { return this->vboIndices; };

            glm::vec3 p1;
            glm::vec3 p2;

            void setSelectionBB(const glm::vec3& p1, const glm::vec3& p2);

        public slots:
			void prepare();

		private:

			SceneGL * sceneGL;

			GLuint program;
			GLuint vao;
			GLuint vboVertices;
			GLuint vboNormals;
			GLuint vboIndices;
		};

	}	 // namespace GL

}	 // namespace UITool

#endif
