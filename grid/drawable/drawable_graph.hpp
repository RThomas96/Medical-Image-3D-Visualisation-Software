#ifndef GL_GRAPH_HPP_
#define GL_GRAPH_HPP_

#include "../../third_party/primitive/Sphere.h"
#include "../../qt/legacy/viewer_structs.hpp"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLFunctions_4_0_Core>

class SceneGL;
class GraphMesh;

namespace UITool {
    //class MeshManipulator;

	/// @defgroup gl GL
	/// @brief All classes that interact with OpenGL. Allow a separation between backend and frontend.
	namespace GL {

        class Graph : QObject {
            Q_OBJECT;
		public:
            Graph(SceneGL* sceneGL, GraphMesh * base);

            void draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat, const glm::vec3& planeDisplacement);
            glm::vec3 lightPosition;

        public slots:
			void prepare();
            void updateManipulatorRadius(float sceneRadius);

        public:

            GraphMesh * graph;
            float manipulatorRatio;
            float manipulatorRadius;

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
