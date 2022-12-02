#ifndef GL_SELECTION_HPP_
#define GL_SELECTION_HPP_

#include <glm/glm.hpp>
#include "glm/gtx/string_cast.hpp"
#include "../../legacy/meshes/drawable/shaders.hpp"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_2_Compatibility>
#include <QOpenGLFunctions_3_2_Core>

class SceneGL;

namespace UITool {
    //class MeshManipulator;

    /// @defgroup gl GL
	/// @brief All classes that interact with OpenGL. Allow a separation between backend and frontend.
	namespace GL {

		class Selection : QObject {
            Q_OBJECT;
		public:
            Selection(ShaderCompiler::GLFunctions* gl, const glm::vec3& p1, const glm::vec3& p2);

			void draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat);

			void setProgram(GLuint program) { this->program = program; };
			GLuint getProgram() { return this->program; };

			void setVao(GLuint vao) { this->vao = vao; };
			GLuint getVao() { return this->vao; };

			void setVboVertices(GLuint vboVertices) { this->vboVertices = vboVertices; };
			GLuint getVboVertices() { return this->vboVertices; };

			void setVboIndices(GLuint vboIndices) { this->vboIndices = vboIndices; };
			GLuint getVboIndices() { return this->vboIndices; };

            glm::vec3 p0;
            glm::vec3 p1;
            glm::vec3 p2;
            glm::vec3 p3;

            glm::vec4 color;

            void setSelectionBB(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
            void setColor(const glm::vec4& color) { this->color = color; };

        public slots:
			void prepare();

		private:

            ShaderCompiler::GLFunctions* sceneGL;

			GLuint program;
			GLuint vao;
			GLuint vboVertices;
			GLuint vboNormals;
			GLuint vboIndices;
		};

	}	 // namespace GL

}	 // namespace UITool

#endif
