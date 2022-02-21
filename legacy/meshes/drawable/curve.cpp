#include "./curve.hpp"

#include <glm/gtc/type_ptr.hpp>

DrawableCurve::DrawableCurve(std::shared_ptr<Curve> &_curve) :
	curve(_curve), DrawableBase() {
	this->vao		   = 0;
	this->vbo_vertices = 0;
	this->vbo_normals  = 0;
	this->vbo_texture  = 0;
	this->vbo_indices  = 0;

	auto curve_bb		   = this->curve->getBB();
	this->bounding_box_min = curve_bb[0];
	this->bounding_box_max = curve_bb[1];
}

void DrawableCurve::initialize(QOpenGLContext *_context, ShaderCompiler::GLFunctions *functions) {
	this->gl			= functions;
	this->bound_context = _context;

	// Create the right shader program :
	auto compiler = std::make_unique<ShaderCompiler>(functions);
	compiler->vertexShader_file("../shaders/base_curve.vert").fragmentShader_file("../shaders/base_curve.frag");
	if (compiler->compileShaders()) {
		this->program_handle_draw = compiler->programName();
	} else {
		std::cerr << "Error while building shaders for drawable curve.\n"
				  << compiler->errorString() << "\n";
	}

	this->makeVAO();

	auto bb = this->curve->getBB();
	for (const auto &bbpoint : bb) {
		this->bounding_box_min = glm::min(bbpoint, this->bounding_box_min);
		this->bounding_box_max = glm::max(bbpoint, this->bounding_box_max);
	}
}

void DrawableCurve::draw(GLfloat *proj_mat, GLfloat *view_mat, glm::vec4 camera) {
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

	this->gl->glUniformMatrix4fv(location_proj, 1, GL_FALSE, proj_mat);
	this->gl->glUniformMatrix4fv(location_view, 1, GL_FALSE, view_mat);
	this->gl->glUniformMatrix4fv(location_model, 1, GL_FALSE, glm::value_ptr(this->transformation_matrix));
	this->gl->glUniform4fv(location_camera_pos, 1, glm::value_ptr(camera));

	// Launch a glDrawElements() command
	this->gl->glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(this->curve->getPositions().size()), GL_UNSIGNED_INT, 0);

	// Unbind the VAO & program
	this->gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	this->gl->glBindVertexArray(0);
	this->gl->glUseProgram(0);
}

void DrawableCurve::fastDraw(GLfloat *proj_mat, GLfloat *view_mat, glm::vec4 camera) {
	this->draw(proj_mat, view_mat, camera);
}

void DrawableCurve::makeVAO(void) {
	// Fetch the curve information :
	auto vertices = this->curve->getPositions();
	std::vector<glm::vec3> normals(vertices.size(), glm::vec3{1.f});
	std::vector<glm::vec2> texture_dummy(vertices.size(), glm::vec2{});
	std::vector<GLuint> final_order(vertices.size());
	for (std::size_t i = 0; i < final_order.size(); ++i) {
		final_order[i] = i;
	}

	// Create the VAO :
	this->gl->glGenVertexArrays(1, &this->vao);

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
	this->gl->glBindVertexArray(this->vao);
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

void DrawableCurve::updateData(void) {
	auto vertices = this->curve->getPositions();
	std::vector<glm::vec3> normals(vertices.size(), glm::vec3{1.f});
	// we assume that the triangles and texture data do not change ...
	this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
	this->gl->glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
	this->gl->glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
	// disable update for new draw call :
	this->should_update_on_next_draw = false;
}
