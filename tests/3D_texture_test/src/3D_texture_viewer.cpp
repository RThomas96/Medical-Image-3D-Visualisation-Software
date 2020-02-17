#include "../include/3D_texture_viewer.hpp"
#include "../include/image_stack_loader.hpp"

#include <GL/glext.h>

void simple_3D_texture_viewer::init() {
	this->restoreStateFromFile();
	this->stack_loader = new image_stack_loader();
	this->setup_cube_attribs();
	this->load_textures();
}

void simple_3D_texture_viewer::load_textures() {
	glEnable(GL_TEXTURE_3D);
	std::cerr << "OpenGL current version : " << glGetString(GL_VERSION) << std::endl;
	std::cerr << "OpenGL shaders version : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cerr << "OpenGL vendor name : " << glGetString(GL_VENDOR) << std::endl;
	std::cerr << "OpenGL renderer : " << glGetString(GL_RENDERER) << std::endl;
	this->texture_data = this->stack_loader->load_stack_from_folder();

	// Generate texture :
	glGenTextures(1, &this->texture_id);
	// Bind it :
	glBindTexture(GL_TEXTURE_3D, this->texture_id);
	// Set the interpolation to be nearest neighbor :
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// Clamp textures to 0;1
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// If above 1, put magenta behind the texture :
	GLfloat border_color[] = {1., .0, 1., 1.};
	glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, border_color);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
	glTexImage3D(
		GL_TEXTURE_3D,						// GLenum : Target
		static_cast<GLint>(0),					// GLint  : Level of detail of the current texture (0 = original)
		GL_DEPTH_COMPONENT,					// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		static_cast<GLsizei>(stack_loader->get_image_width()),	// GLsizei: Image width
		static_cast<GLsizei>(stack_loader->get_image_height()),	// GLsizei: Image height
		static_cast<GLsizei>(stack_loader->get_image_depth()),	// GLsizei: Image depth (number of layers)
		static_cast<GLint>(0),					// GLint  : Border. This value MUST be 0.
		GL_DEPTH_COMPONENT,					// GLenum : Format of the pixel data
		GL_UNSIGNED_BYTE,					// GLenum : Type (the data type as in uchar, uint, float ...)
		this->texture_data.data()				// void*  : Data to load into the buffer
	);
}

void simple_3D_texture_viewer::draw() {
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, this->texture_id);

	/**
	 * Cube order :
	 *        Y
	 *        |
	 *        |
	 *       v6----- v5
	 *      /|      /|
	 *     v1------v0|
	 *     | |     | |
	 *     | |v7---|-|v4 --> X
	 *     |/      |/
	 *     v2------v3
	 *   /
	 *  L
	 * Z
	 */

	glBegin(GL_TRIANGLES);
	{
		this->draw_face_gl(0, 1, 2);
		this->draw_face_gl(2, 3, 0);

		this->draw_face_gl(5, 0, 3);
		this->draw_face_gl(3, 4, 5);

		this->draw_face_gl(6, 5, 4);
		this->draw_face_gl(4, 7, 6);

		this->draw_face_gl(1, 6, 7);
		this->draw_face_gl(7, 2, 1);

		this->draw_face_gl(5, 6, 1);
		this->draw_face_gl(1, 0, 5);

		this->draw_face_gl(3, 2, 7);
		this->draw_face_gl(7, 4, 3);
	}
	glEnd();
}

void simple_3D_texture_viewer::setup_cube_attribs() {
	// a cube has 6 vertices (modify here for more complicated than cube)
	this->vertex_count = 8;
	// setup vertices and normals :
	this->vertex_pos = new qreal [this->vertex_count * 3];
	this->vertex_nor = new qreal [this->vertex_count * 3];
	this->vertex_tex = new qreal*[this->vertex_count * 3];
	// define max and min coords for texture application
	this->tex_coords_min = new qreal[3];
	this->tex_coords_max = new qreal[3];

	this->tex_coords_min[0] = 0.0;
	this->tex_coords_min[1] = 0.0;
	this->tex_coords_min[2] = 0.0;

	this->tex_coords_max[0] = 1.0;
	this->tex_coords_max[1] = 1.0;
	this->tex_coords_max[2] = 1.0;

	/**
	 * Cube order :
	 *        Y
	 *        |
	 *        |
	 *       v6----- v5
	 *      /|      /|
	 *     v1------v0|
	 *     | |     | |
	 *     | |v7---|-|v4 --> X
	 *     |/      |/
	 *     v2------v3
	 *   /
	 *  L
	 * Z
	 *
	 * Warning : very dirty code ahead
	 */

	// v0 :
	this->vertex_pos[ 0] = 1.0;
	this->vertex_pos[ 1] = 1.0;
	this->vertex_pos[ 2] = 1.0;
	// v1 :
	this->vertex_pos[ 3] =-1.0;
	this->vertex_pos[ 4] = 1.0;
	this->vertex_pos[ 5] = 1.0;
	// v2 :
	this->vertex_pos[ 6] =-1.0;
	this->vertex_pos[ 7] =-1.0;
	this->vertex_pos[ 8] = 1.0;
	// v3 :
	this->vertex_pos[ 9] = 1.0;
	this->vertex_pos[10] =-1.0;
	this->vertex_pos[11] = 1.0;
	// v4 :
	this->vertex_pos[12] = 1.0;
	this->vertex_pos[13] =-1.0;
	this->vertex_pos[14] =-1.0;
	// v5 :
	this->vertex_pos[15] = 1.0;
	this->vertex_pos[16] = 1.0;
	this->vertex_pos[17] =-1.0;
	// v6 :
	this->vertex_pos[18] =-1.0;
	this->vertex_pos[19] = 1.0;
	this->vertex_pos[20] =-1.0;
	// v7 :
	this->vertex_pos[21] =-1.0;
	this->vertex_pos[22] =-1.0;
	this->vertex_pos[23] =-1.0;

	qreal denom = std::sqrt(3.0);

	// v0 :
	this->vertex_nor[ 0] = 1.0 / denom;
	this->vertex_nor[ 1] = 1.0 / denom;
	this->vertex_nor[ 2] = 1.0 / denom;
	// v1 :
	this->vertex_nor[ 3] =-1.0 / denom;
	this->vertex_nor[ 4] = 1.0 / denom;
	this->vertex_nor[ 5] = 1.0 / denom;
	// v2 :
	this->vertex_nor[ 6] =-1.0 / denom;
	this->vertex_nor[ 7] =-1.0 / denom;
	this->vertex_nor[ 8] = 1.0 / denom;
	// v3 :
	this->vertex_nor[ 9] = 1.0 / denom;
	this->vertex_nor[10] =-1.0 / denom;
	this->vertex_nor[11] = 1.0 / denom;
	// v4 :
	this->vertex_nor[12] = 1.0 / denom;
	this->vertex_nor[13] =-1.0 / denom;
	this->vertex_nor[14] =-1.0 / denom;
	// v5 :
	this->vertex_nor[15] = 1.0 / denom;
	this->vertex_nor[16] = 1.0 / denom;
	this->vertex_nor[17] =-1.0 / denom;
	// v6 :
	this->vertex_nor[18] =-1.0 / denom;
	this->vertex_nor[19] = 1.0 / denom;
	this->vertex_nor[20] =-1.0 / denom;
	// v7 :
	this->vertex_nor[21] =-1.0 / denom;
	this->vertex_nor[22] =-1.0 / denom;
	this->vertex_nor[23] =-1.0 / denom;

	// v0 :
	this->vertex_tex[ 0] = &this->tex_coords_max[0];
	this->vertex_tex[ 1] = &this->tex_coords_max[1];
	this->vertex_tex[ 2] = &this->tex_coords_max[2];
	// v1 :
	this->vertex_tex[ 3] = &this->tex_coords_min[0];
	this->vertex_tex[ 4] = &this->tex_coords_max[1];
	this->vertex_tex[ 5] = &this->tex_coords_max[2];
	// v2 :
	this->vertex_tex[ 6] = &this->tex_coords_min[0];
	this->vertex_tex[ 7] = &this->tex_coords_min[1];
	this->vertex_tex[ 8] = &this->tex_coords_max[2];
	// v3 :
	this->vertex_tex[ 9] = &this->tex_coords_max[0];
	this->vertex_tex[10] = &this->tex_coords_min[1];
	this->vertex_tex[11] = &this->tex_coords_max[2];
	// v4 :
	this->vertex_tex[12] = &this->tex_coords_max[0];
	this->vertex_tex[13] = &this->tex_coords_min[1];
	this->vertex_tex[14] = &this->tex_coords_min[2];
	// v5 :
	this->vertex_tex[15] = &this->tex_coords_max[0];
	this->vertex_tex[16] = &this->tex_coords_max[1];
	this->vertex_tex[17] = &this->tex_coords_min[2];
	// v6 :
	this->vertex_tex[18] = &this->tex_coords_min[0];
	this->vertex_tex[19] = &this->tex_coords_max[1];
	this->vertex_tex[20] = &this->tex_coords_min[2];
	// v7 :
	this->vertex_tex[21] = &this->tex_coords_min[0];
	this->vertex_tex[22] = &this->tex_coords_min[1];
	this->vertex_tex[23] = &this->tex_coords_min[2];
}

void simple_3D_texture_viewer::draw_face_gl(std::size_t v0, std::size_t v1, std::size_t v2) {
	glTexCoord3d(*this->vertex_tex[v0 * 3 + 0], *this->vertex_tex[v0 * 3 + 1], *this->vertex_tex[v0 * 3 + 2]);
	glNormal3d(this->vertex_nor[v0 * 3 + 0], this->vertex_nor[v0 * 3 + 1], this->vertex_nor[v0 * 3 + 2]);
	glVertex3d(this->vertex_pos[v0 * 3 + 0], this->vertex_pos[v0 * 3 + 1], this->vertex_pos[v0 * 3 + 2]);

	glTexCoord3d(*this->vertex_tex[v1 * 3 + 0], *this->vertex_tex[v1 * 3 + 1], *this->vertex_tex[v1 * 3 + 2]);
	glNormal3d(this->vertex_nor[v1 * 3 + 0], this->vertex_nor[v1 * 3 + 1], this->vertex_nor[v1 * 3 + 2]);
	glVertex3d(this->vertex_pos[v1 * 3 + 0], this->vertex_pos[v1 * 3 + 1], this->vertex_pos[v1 * 3 + 2]);

	glTexCoord3d(*this->vertex_tex[v2 * 3 + 0], *this->vertex_tex[v2 * 3 + 1], *this->vertex_tex[v2 * 3 + 2]);
	glNormal3d(this->vertex_nor[v2 * 3 + 0], this->vertex_nor[v2 * 3 + 1], this->vertex_nor[v2 * 3 + 2]);
	glVertex3d(this->vertex_pos[v2 * 3 + 0], this->vertex_pos[v2 * 3 + 1], this->vertex_pos[v2 * 3 + 2]);
}
































