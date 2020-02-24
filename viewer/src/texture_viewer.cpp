#include "../include/texture_viewer.hpp"

// Define the voxel depth (distance between 2 slices)
#define VOXEL_DEPTH 1.927
#define VOXEL_SIDE 0.39
// Taken from WolframAlpha for the exact fraction :
#define VOXEL_RATIO 1927.0/399360.0
#define VOXEL_RATIO_HALF 1927.0/798720.0

void texture_viewer::init() {
	this->restoreStateFromFile();
	this->stack_loader = nullptr;
	this->print_opengl_info();
	this->setup_cube_attribs();

	// Enable the use of 3D textures
	// (needed, even if none are loaded now)
	// (enabling it later doesn't work)
	glEnable(GL_TEXTURE_3D);
}

void texture_viewer::print_opengl_info() const {
	std::cerr << "OpenGL current version : " << glGetString(GL_VERSION) << std::endl;
	std::cerr << "OpenGL shaders version : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cerr << "OpenGL vendor name : " << glGetString(GL_VENDOR) << std::endl;
	std::cerr << "OpenGL renderer : " << glGetString(GL_RENDERER) << std::endl;
}

void texture_viewer::load_textures() {
	const unsigned char* texture_data = this->stack_loader->load_stack_from_folder();

	if (texture_data == nullptr) {
		std::cerr << "nullptr returned.\nNo image data was loaded." << std::endl;
	}

	// Generate texture :
	glGenTextures(1, &this->texture_id);
	// Bind it :
	glBindTexture(GL_TEXTURE_3D, this->texture_id);

	// Set the interpolation to be nearest neighbor :
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// Try to reduce the number of mipmaps produced by OpenGL (experimental)
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, static_cast<GLfloat>(-1000.0));

	// Clamp textures to 0;1
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Set the R, G, B components to take the R channel value, and 0 for alpha
	GLint swizzle_mask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
	glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask);

	glTexImage3D(
		GL_TEXTURE_3D,						// GLenum : Target
		static_cast<GLint>(0),					// GLint  : Level of detail of the current texture (0 = original)
		GL_RED,							// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		static_cast<GLsizei>(stack_loader->get_image_width()),	// GLsizei: Image width
		static_cast<GLsizei>(stack_loader->get_image_height()),	// GLsizei: Image height
		static_cast<GLsizei>(stack_loader->get_image_depth()),	// GLsizei: Image depth (number of layers)
		static_cast<GLint>(0),					// GLint  : Border. This value MUST be 0.
		GL_RED,							// GLenum : Format of the pixel data
		GL_UNSIGNED_BYTE,					// GLenum : Type (the data type as in uchar, uint, float ...)
		texture_data						// void*  : Data to load into the buffer
	);

	this->set_real_voxel_dimensions(stack_loader->get_image_depth());

	// we can now safely ask the loader to delete the data
	this->stack_loader->free_data();
}

void texture_viewer::draw() {
	// disable lighting computation
	glDisable(GL_LIGHTING);
	// check if the texture has been created :
	if (glIsTexture(this->texture_id) == GL_TRUE) {
		// bind texture to current context
		glBindTexture(GL_TEXTURE_3D, this->texture_id);
	}

	// See cube order in the setup_cube_attribs() function.

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

void texture_viewer::setup_cube_attribs() {
	// setup vertices and normals (8 verts * 3 coords) :
	this->vertex_pos = new qreal [8 * 3];
	this->vertex_nor = new qreal [8 * 3];
	this->vertex_tex = new qreal*[8 * 3];
	// define max and min coords for texture application
	this->tex_coords_min = new qreal[3];
	this->tex_coords_max = new qreal[3];

	this->tex_coords_min[0] = 0.0;
	this->tex_coords_min[1] = 0.0;
	this->tex_coords_min[2] = 0.0;

	this->tex_coords_max[0] = 0.99;
	this->tex_coords_max[1] = 0.99;
	this->tex_coords_max[2] = 0.99;

	/**
	 * Cube order for the class :
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

void texture_viewer::draw_face_gl(std::size_t v0, std::size_t v1, std::size_t v2) {
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

void texture_viewer::set_min_X_tex_value(double x) {
	this->tex_coords_min[0] = x;
	this->set_back_x_plane_coordinates();
	this->update();
}

void texture_viewer::set_min_Y_tex_value(double y) {
	this->tex_coords_min[1] = y;
	this->set_back_y_plane_coordinates();
	this->update();
}

void texture_viewer::set_min_Z_tex_value(double z) {
	this->tex_coords_min[2] = z;
	this->set_back_z_plane_coordinates();
	this->update();
}

void texture_viewer::set_max_X_tex_value(double x) {
	this->tex_coords_max[0] = x;
	this->set_front_x_plane_coordinates();
	this->update();
}

void texture_viewer::set_max_Y_tex_value(double y) {
	this->tex_coords_max[1] = y;
	this->set_front_y_plane_coordinates();
	this->update();
}

void texture_viewer::set_max_Z_tex_value(double z) {
	this->tex_coords_max[2] = z;
	this->set_front_z_plane_coordinates();
	this->update();
}

void texture_viewer::set_back_x_plane_coordinates() {
	this->vertex_pos[ 1*3 + 0 ] = (this->tex_coords_min[0] * 2.0) - 1.0;
	this->vertex_pos[ 2*3 + 0 ] = (this->tex_coords_min[0] * 2.0) - 1.0;
	this->vertex_pos[ 6*3 + 0 ] = (this->tex_coords_min[0] * 2.0) - 1.0;
	this->vertex_pos[ 7*3 + 0 ] = (this->tex_coords_min[0] * 2.0) - 1.0;
}

void texture_viewer::set_front_x_plane_coordinates() {
	this->vertex_pos[ 0*3 + 0 ] = (this->tex_coords_max[0] * 2.0) - 1.0;
	this->vertex_pos[ 3*3 + 0 ] = (this->tex_coords_max[0] * 2.0) - 1.0;
	this->vertex_pos[ 4*3 + 0 ] = (this->tex_coords_max[0] * 2.0) - 1.0;
	this->vertex_pos[ 5*3 + 0 ] = (this->tex_coords_max[0] * 2.0) - 1.0;
}

void texture_viewer::set_back_y_plane_coordinates() {
	this->vertex_pos[ 2*3 + 1 ] = (this->tex_coords_min[1] * 2.0) - 1.0;
	this->vertex_pos[ 3*3 + 1 ] = (this->tex_coords_min[1] * 2.0) - 1.0;
	this->vertex_pos[ 4*3 + 1 ] = (this->tex_coords_min[1] * 2.0) - 1.0;
	this->vertex_pos[ 7*3 + 1 ] = (this->tex_coords_min[1] * 2.0) - 1.0;
}

void texture_viewer::set_front_y_plane_coordinates() {
	this->vertex_pos[ 0*3 + 1 ] = (this->tex_coords_max[1] * 2.0) - 1.0;
	this->vertex_pos[ 1*3 + 1 ] = (this->tex_coords_max[1] * 2.0) - 1.0;
	this->vertex_pos[ 5*3 + 1 ] = (this->tex_coords_max[1] * 2.0) - 1.0;
	this->vertex_pos[ 6*3 + 1 ] = (this->tex_coords_max[1] * 2.0) - 1.0;
}

void texture_viewer::set_back_z_plane_coordinates() {
	this->vertex_pos[ 4*3 + 2 ] = (this->tex_coords_min[2] * 2.0) - 1.0;
	this->vertex_pos[ 5*3 + 2 ] = (this->tex_coords_min[2] * 2.0) - 1.0;
	this->vertex_pos[ 6*3 + 2 ] = (this->tex_coords_min[2] * 2.0) - 1.0;
	this->vertex_pos[ 7*3 + 2 ] = (this->tex_coords_min[2] * 2.0) - 1.0;
}

void texture_viewer::set_front_z_plane_coordinates() {
	this->vertex_pos[ 0*3 + 2 ] = (this->tex_coords_max[2] * 2.0) - 1.0;
	this->vertex_pos[ 1*3 + 2 ] = (this->tex_coords_max[2] * 2.0) - 1.0;
	this->vertex_pos[ 2*3 + 2 ] = (this->tex_coords_max[2] * 2.0) - 1.0;
	this->vertex_pos[ 3*3 + 2 ] = (this->tex_coords_max[2] * 2.0) - 1.0;
}

void texture_viewer::set_to_unit_cube() {
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
}

void texture_viewer::set_real_voxel_dimensions(size_t nb_images_loaded) {
	// The farthest point, when not centralised :
	qreal far = static_cast<qreal>(nb_images_loaded) * VOXEL_RATIO;
	qreal near = .0;
	this->vertex_pos[ 2] = far - far/2.0; // v0
	this->vertex_pos[ 5] = far - far/2.0; // v1
	this->vertex_pos[ 8] = far - far/2.0; // v2
	this->vertex_pos[11] = far - far/2.0; // v3
	this->vertex_pos[14] =near - far/2.0; // v4
	this->vertex_pos[17] =near - far/2.0; // v5
	this->vertex_pos[20] =near - far/2.0; // v6
	this->vertex_pos[23] =near - far/2.0; // v7
	this->setSceneRadius(std::max(std::sqrt(3.0),std::sqrt(1.+1.+std::pow(far/2.0, 2.0))));
}

void texture_viewer::request_texture_load() {
	if (this->stack_loader == nullptr) {
		this->stack_loader = new bulk_texture_loader();
		this->stack_loader->enable_downsampling(true); // can downsample, don't need full-res for viewing
	} else {
		this->request_texture_deletion();
	}
	this->load_textures();
	this->update();
}

void texture_viewer::request_texture_deletion() {
	this->set_to_unit_cube();
	this->setSceneRadius(std::sqrt(3.0));
	glBindTexture(GL_TEXTURE_3D, this->texture_id);
	// define a shortcut to static_cast here, to make the line shorter
	#define cast static_cast<GLsizei>
	glTexImage3D(GL_TEXTURE_3D, static_cast<GLint>(0), GL_RED, cast(0), cast(0), cast(0), cast(0), GL_RED, GL_UNSIGNED_BYTE, nullptr);
	// remove shortcut
	#undef cast
	glDeleteTextures(1, &this->texture_id);
	this->update();
}
