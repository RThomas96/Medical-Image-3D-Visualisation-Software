#include "../include/volumetric_grid_viewer.hpp"

#include "../include/scene.hpp"

GridViewer::GridViewer(Image::Grid::Ptr& _grid_to_show) {
	// For now, try to display the whole grid.
	// TODO: Downsample the grid given in argument here.

	// By default, the grid is hidden because the data is not yet initialized.
	this->is_grid_hidden = true;
	this->grid_texture = GL_INVALID_INDEX;
	this->ubo_handle = GL_INVALID_INDEX;
	this->texture_handles = VolMesh{};
	this->main_channel_index = 0;
	this->source_grid = _grid_to_show;
	this->volumetric_epsilon = glm::vec3{.0f, .0f, .0f};

	// Let the initialization of the color channel data to the initialization functions.
}

GridViewer::~GridViewer(void) {
	// Nothing for now, all cleans up naturally.
}

void GridViewer::initializeData() {
	// First, signal we're before CPU init
	emit this->beforeCPUInit();

	// Grid is supposed to be set in the ctor and displayable (small enough to fit in memory)
	this->generateMeshData();

	// Finally, signal CPU init has ended.
	emit this->afterCPUInit();
}

void GridViewer::initializeGLData(Scene *_scene) {
	emit this->beforeGLInit();

	// Get grid dimensions :
	glm::tvec4<std::size_t> dimensions{this->source_grid->getResolution(), this->source_grid->getVoxelDimensionality()};

	// Upload the texture to the GPU :
	TextureUpload _gridTex{};
	_gridTex.minmag.x = GL_NEAREST;
	_gridTex.minmag.y = GL_NEAREST;
	_gridTex.lod.y = -1000.f;
	_gridTex.wrap.x = GL_CLAMP_TO_EDGE;
	_gridTex.wrap.y = GL_CLAMP_TO_EDGE;
	_gridTex.wrap.z = GL_CLAMP_TO_EDGE;
	_gridTex.swizzle.r = GL_RED;
	if (dimensions.a > 1) { _gridTex.swizzle.g = GL_GREEN; } else { _gridTex.swizzle.g = GL_ZERO; }
	if (dimensions.a > 2) { _gridTex.swizzle.b = GL_BLUE ; } else { _gridTex.swizzle.b = GL_ZERO; }
	if (dimensions.a > 3) { _gridTex.swizzle.a = GL_ALPHA; } else { _gridTex.swizzle.a = GL_ONE ; }
	_gridTex.alignment.x = 1;
	_gridTex.alignment.y = 2;
	switch (dimensions.a) {
		case 1: _gridTex.format = GL_RED_INTEGER ; _gridTex.internalFormat = GL_R16UI   ; break;
		case 2: _gridTex.format = GL_RG_INTEGER  ; _gridTex.internalFormat = GL_RG16UI  ; break;
		case 3: _gridTex.format = GL_RGB_INTEGER ; _gridTex.internalFormat = GL_RGB16UI ; break;
		case 4: _gridTex.format = GL_RGBA_INTEGER; _gridTex.internalFormat = GL_RGBA16UI; break;
	}
	_gridTex.type = GL_UNSIGNED_SHORT;
	_gridTex.size.x = dimensions.x;
	_gridTex.size.y = dimensions.y;
	_gridTex.size.z = dimensions.z;

	std::vector<std::uint16_t> slices(dimensions.x * dimensions.y * dimensions.a);
	this->grid_texture = _scene->newAPI_uploadTexture3D_allocateonly(_gridTex);

	for (std::size_t s = 0; s < dimensions.z; ++s) {
		if (this->source_grid->readSlice(s, slices)) {
			_scene->newAPI_uploadTexture3D(this->grid_texture, _gridTex, s, slices);
		} else {
			std::cerr << "Scene texture upload : Could not read the data at index " << s << " !\n";
		}
	}

	// Create the UBO :
	this->ubo_handle = _scene->createUniformBuffer(4*sizeof(colorChannelAttributes_GL), GL_STATIC_DRAW);
	std::size_t color_s = sizeof(colorChannelAttributes);
	auto& main_channel_data = this->colorChannelAttributes[this->main_channel_index];
	// Upload data : main channel, then red, green, and blue all at once
	_scene->setUniformBufferData(this->ubo_handle, 0*color_s, color_s, &main_channel_data);
	// offset by one sizeof(colorChannelAttributes_GL) and upload 3 times that amount at once
	_scene->setUniformBufferData(this->ubo_handle, color_s, 3*color_s, this->colorChannelAttributes.data());

	// Upload the volumetric mesh's data

	emit this->afterGLInit();
}

void GridViewer::draw3D(Scene *_scene_functions, GLfloat* view_matrix, GLfloat* projection_matrix) {
	if (this->vis_mode == VisualizationMode::Solid) {
		this->draw_solid(_scene_functions, view_matrix, projection_matrix);
	} else if (this->vis_mode == VisualizationMode::Volumetric) {
		this->draw_volumetric(_scene_functions, view_matrix, projection_matrix);
	} else if (this->vis_mode == VisualizationMode::VolumetricBoxed) {
		this->draw_volumetric_boxed(_scene_functions, view_matrix, projection_matrix);
	} else {
		std::cerr << "Error : no drawing mode defined for grid " << this->source_grid->getImageName() << '\n';
	}
}

void GridViewer::draw_solid(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix) {
	// Get the program handle to draw and use it :
	GLuint _program_handle = _scene->getSolidProgram();
	_scene->glUseProgram(_program_handle);

	// Bind the necessary uniforms and the UBO :
	this->bindUniforms_Solid(_scene, view_matrix, projection_matrix);
	this->bindTextures_Solid(_scene);
	this->bindUniformBuffer(_scene, _program_handle, "ColorBlock");

	// Bind the VAO and draw it !
}

void GridViewer::draw_volumetric(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix) {
	// Get the program handle to draw and use it :
	GLuint _program_handle = _scene->getVolumetricProgram();
	_scene->glUseProgram(_program_handle);

	// Bind the necessary uniforms and the UBO :
	this->bindUniforms_Volumetric(_scene, view_matrix, projection_matrix);
	this->bindTextures_Volumetric(_scene);
	this->bindUniformBuffer(_scene, _program_handle, "ColorBlock");

	// Bind the VAO and draw it !
}

void GridViewer::draw_volumetric_boxed(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix) {
	// Get the program handle to draw and use it :
	GLuint _program_handle = _scene->getVolumetricProgram();
	_scene->glUseProgram(_program_handle);

	// Bind the necessary uniforms and the UBO :
	this->bindUniforms_VolumetricBoxed(_scene, view_matrix, projection_matrix);
	this->bindTextures_VolumetricBoxed(_scene);
	this->bindUniformBuffer(_scene, _program_handle, "ColorBlock");

	// Bind the VAO and draw it !
}

void GridViewer::bindUniformBuffer(Scene* _scene, GLuint _program_handle, const char* uniform_buffer_name) {
	GLuint block_index = _scene->glGetUniformBlockIndex(_program_handle, uniform_buffer_name);
	_scene->glUniformBlockBinding(_program_handle, block_index, 0);
	_scene->glBindBufferBase(GL_UNIFORM_BUFFER, 0, this->ubo_handle);
}

void GridViewer::bindUniforms_Solid(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix) {
	//
}

void GridViewer::bindUniforms_Volumetric(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix) {
	//
}

void GridViewer::bindUniforms_VolumetricBoxed(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix) {
	//
}

void GridViewer::updateMainChannel_UBO(Scene* _scene) {
	// If the data was not already set and created, do nothing
	if (this->ubo_handle == GL_INVALID_INDEX) { return; }

	// Get the right data, and upload it to the UBO :
	auto main_channel_data = this->colorChannelAttributes[this->main_channel_index];
	_scene->setUniformBufferData(this->ubo_handle, 0, sizeof(colorChannelAttributes_GL), &main_channel_data);
	// N.B. : main color channel is always in first, so offset from start is 0

	return;
}

void GridViewer::generateMeshData() {
	// typedef for bounding box's vector type :
	using vec_t = typename DiscreteGrid::bbox_t::vec;

	//Min and diagonal of the bounding box (used for position computation) :
	const vec_t min = this->source_grid->getBoundingBox().getMin();
	const vec_t diag = this->source_grid->getBoundingBox().getDiagonal();
	// Dimensions, subject to change :
	std::size_t xv = 10 ; glm::vec4::value_type xs = diag.x / static_cast<glm::vec4::value_type>(xv);
	std::size_t yv = 10 ; glm::vec4::value_type ys = diag.y / static_cast<glm::vec4::value_type>(yv);
	std::size_t zv = 10 ; glm::vec4::value_type zs = diag.z / static_cast<glm::vec4::value_type>(zv);

	// Size of tetrahedra, to compute epsilon :
	glm::vec3 epsVolu = glm::vec3(xs, ys, zs);
	this->volumetric_epsilon = epsVolu;

	// Containers for the computation of positions and texture coordinates :
	glm::vec4 pos = glm::vec4();
	glm::vec3 tex = glm::vec3();
	// Transformation to apply to the mesh :
	MatrixTransform::Ptr grid_transform_pointer =
			std::dynamic_pointer_cast<MatrixTransform>(this->source_grid->getPrecomputedMatrix());

	// If the std::dynamic_pointer_cast<>() failed, replace with a default transform :
	if (grid_transform_pointer == nullptr) {
		std::cerr << "Warning : the grid transform couldn't be transformed to a matric transform.\n";
		std::cerr << "Warning : Transform is thus set to a default transform.\n";
		grid_transform_pointer = std::make_shared<DefaultTransform>();
	}
	// Get the matrix transform of the grid (or the default transform in case an error happened) :
	glm::mat4 transfo = grid_transform_pointer->matrix();

	using val_t = vec_t::value_type;

	// Create vertices along with their texture coordinates. We
	// need to go one after because we want _n_ tetrahedra, and
	// thus must finish the last 'row'/'column' of tetrahedra :
	for (std::size_t k = 0; k <= zv; ++k) {
		for (std::size_t j = 0; j <= yv; ++j) {
			for (std::size_t i = 0; i <= xv; ++i) {
				pos = transfo * glm::vec4(min.x + static_cast<val_t>(i)*xs, min.y + static_cast<val_t>(j)*ys,
										  min.z + static_cast<val_t>(k)*zs, 1.f);
				//
				tex = glm::vec3(
					static_cast<val_t>(i)/static_cast<val_t>(xv),
					static_cast<val_t>(j)/static_cast<val_t>(yv),
					static_cast<val_t>(k)/static_cast<val_t>(zv)
				);

				this->volumetric_mesh.positions.push_back(pos);
				this->volumetric_mesh.texture.push_back(tex);
			}
		}
	}

	// Return position for vertex generated at indices I, J, K :
	std::size_t xt = xv+1; std::size_t yt = yv+1;
	auto getIndice = [&, xt, yt](std::size_t i, std::size_t j, std::size_t k) -> std::size_t {
		return i + j * xt + k * xt * yt;
	};

	// Create tetrahedra :
	for (std::size_t k = 0; k < zv; ++k) {
		for (std::size_t j = 0; j < yv; ++j) {
			for (std::size_t i = 0; i < xv; ++i) {
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i+1, j  , k  ), getIndice(i+1, j+1, k  ), getIndice(i  , j+1, k  ), getIndice(i+1, j+1, k+1)});
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i  , j  , k+1), getIndice(i  , j  , k  ), getIndice(i  , j+1, k+1), getIndice(i+1, j  , k+1)});
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i  , j+1, k+1), getIndice(i+1, j  , k  ), getIndice(i+1, j+1, k+1), getIndice(i+1, j  , k+1)});
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i  , j  , k  ), getIndice(i+1, j  , k  ), getIndice(i  , j+1, k+1), getIndice(i+1, j  , k+1)});
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i  , j  , k  ), getIndice(i+1, j  , k  ), getIndice(i  , j+1, k  ), getIndice(i  , j+1, k+1)});
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i  , j+1, k  ), getIndice(i+1, j  , k  ), getIndice(i+1, j+1, k+1), getIndice(i  , j+1, k+1)});
			}
		}
	}

	/* Mesh is now built, and should conform to the grid well enough.
	 * Some artefacts might appear at some orientations, due to floating
	 * point imprecision in the DDA tracing. They will be along the edge
	 * of the mesh's tetrahedra. */

	return;
}
