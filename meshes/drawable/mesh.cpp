#include "./mesh.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <memory>

DrawableMesh::DrawableMesh(std::shared_ptr<Mesh> &_mesh) :
	mesh(_mesh), DrawableBase() {
	this->vao		   = 0;
	this->vbo_vertices = 0;
	this->vbo_normals  = 0;
	this->vbo_texture  = 0;
	this->vbo_indices  = 0;

	this->program_handle_draw = 0;
	this->program_handle_fastdraw = 0;

	this->transformation_matrix = glm::mat4(1.f);

	auto mesh_bb		   = this->mesh->getBB();
	this->bounding_box_min = mesh_bb[0];
	this->bounding_box_max = mesh_bb[1];
}

DrawableMesh::~DrawableMesh() noexcept {
	this->gl->glDeleteBuffers(1, &this->vbo_vertices);
	this->gl->glDeleteBuffers(1, &this->vbo_normals);
	this->gl->glDeleteBuffers(1, &this->vbo_texture);
	this->gl->glDeleteBuffers(1, &this->vbo_indices);
	this->gl->glDeleteVertexArrays(1, &this->vao);
	this->gl->glDeleteProgram(this->program_handle_draw);
	this->gl->glDeleteProgram(this->program_handle_fastdraw);
}

void DrawableMesh::initialize(QOpenGLContext *_context, ShaderCompiler::GLFunctions *functions) {
	this->gl			= functions;
	this->bound_context = _context;

	std::cerr << "Initializing mesh from " << this->mesh->getVertices().size() << " vertices in the mesh.\n";

	// Create the right shader program :
	auto compiler = std::make_unique<ShaderCompiler>(functions);
	compiler->vertexShader_file("../new_shaders/base_mesh.vert").fragmentShader_file("../new_shaders/base_mesh.frag");
	if (compiler->compileShaders()) {
		this->program_handle_draw = compiler->programName();
	} else {
		std::cerr << "Error while building shaders for drawable mesh.\n"
				  << compiler->errorString() << "\n";
	}

	this->makeVAO();

	auto bb = this->mesh->getBB();
	for (const auto &bbpoint : bb) {
		this->bounding_box_min = glm::min(bbpoint, this->bounding_box_min);
		this->bounding_box_max = glm::max(bbpoint, this->bounding_box_max);
	}
}

void DrawableMesh::draw(GLfloat *proj_mat, GLfloat *view_mat, glm::vec4 camera) {
	if (this->should_update_on_next_draw) {
		this->updateData();
	}

	// Bind the VAO & program
	this->gl->glUseProgram(this->program_handle_draw);
	this->gl->glBindVertexArray(this->vao);
	this->gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_indices);

	GLint location_proj		  = this->gl->glGetUniformLocation(this->program_handle_draw, "proj");
	GLint location_view		  = this->gl->glGetUniformLocation(this->program_handle_draw, "view");
	GLint location_model	  = this->gl->glGetUniformLocation(this->program_handle_draw, "model");
	GLint location_camera_pos = this->gl->glGetUniformLocation(this->program_handle_draw, "camera_pos");

	GLint location_bb_min = this->gl->glGetUniformLocation(this->program_handle_draw, "bb_min");
	GLint location_bb_max = this->gl->glGetUniformLocation(this->program_handle_draw, "bb_max");

	this->gl->glUniformMatrix4fv(location_proj, 1, GL_FALSE, proj_mat);
	this->gl->glUniformMatrix4fv(location_view, 1, GL_FALSE, view_mat);
	this->gl->glUniformMatrix4fv(location_model, 1, GL_FALSE, glm::value_ptr(this->transformation_matrix));
	this->gl->glUniform4fv(location_camera_pos, 1, glm::value_ptr(camera));

	this->gl->glUniform4f(location_bb_min, this->bounding_box_min.x, this->bounding_box_min.y, this->bounding_box_min.z, 1.f);
	this->gl->glUniform4f(location_bb_max, this->bounding_box_max.x, this->bounding_box_max.y, this->bounding_box_max.z, 1.f);

	// Launch a glDrawElements() command
	this->gl->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->mesh->getTriangles().size() * 3), GL_UNSIGNED_INT, 0);

	// Unbind the VAO & program
	this->gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	this->gl->glBindVertexArray(0);
	this->gl->glUseProgram(0);
}

void DrawableMesh::fastDraw(GLfloat *proj_mat, GLfloat *view_mat, glm::vec4 camera) {
	this->draw(proj_mat, view_mat, camera);
}

void DrawableMesh::updateBoundingBox() {
	auto bb = this->mesh->getBB();
	this->bounding_box_min = bb[0];
	this->bounding_box_max = bb[1];
}

void DrawableMesh::makeVAO(void) {
	// Fetch the mesh information :
	auto vertices = this->mesh->getVertices();
	auto normals  = this->mesh->getVertexNormals();
	std::vector<glm::vec2> texture_dummy(vertices.size(), glm::vec2{});
	std::vector<GLuint> final_order;	// indices to draw the mesh with
	for (const auto &triangle : this->mesh->getTriangles()) {
		final_order.emplace_back(triangle.getVertex(0));
		final_order.emplace_back(triangle.getVertex(1));
		final_order.emplace_back(triangle.getVertex(2));
	}

	// Create the VAO :
	this->gl->glGenVertexArrays(1, &this->vao);
	this->gl->glBindVertexArray(this->vao);

	// Create the VBOs :

	// vertex buffer :
	this->gl->glGenBuffers(1, &this->vbo_vertices);
	this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
	this->gl->glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	// normal buffer :
	this->gl->glGenBuffers(1, &this->vbo_normals);
	this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
	this->gl->glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
	// texture buffer :
	this->gl->glGenBuffers(1, &this->vbo_texture);
	this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_texture);
	this->gl->glBufferData(GL_ARRAY_BUFFER, texture_dummy.size() * 2 * sizeof(GLfloat), texture_dummy.data(), GL_STATIC_DRAW);

	// index buffer :
	this->gl->glGenBuffers(1, &this->vbo_indices);
	this->gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_indices);
	this->gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(final_order.size()) * sizeof(GLuint), final_order.data(), GL_STATIC_DRAW);

	// Populate the VAO :
	// layout(location=0) : vertices (vec3)
	this->gl->glEnableVertexAttribArray(0);
	this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
	this->gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
	// layout(location=1) : normals (vec3)
	this->gl->glEnableVertexAttribArray(1);
	this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
	this->gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
	// layout(location=2) : textures (vec2)
	this->gl->glEnableVertexAttribArray(2);
	this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_texture);
	this->gl->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
}

void DrawableMesh::updateData(void) {
	auto vertices = this->mesh->getVertices();
	auto normals  = this->mesh->getVertexNormals();
	// we assume that the triangles and texture data do not change ...
	this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
	this->gl->glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * 3 * sizeof(GLfloat), vertices.data());
	this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
	this->gl->glBufferSubData(GL_ARRAY_BUFFER, 0, normals.size() * 3 * sizeof(GLfloat), normals.data());
	// disable update for new draw call :
	this->should_update_on_next_draw = false;
}