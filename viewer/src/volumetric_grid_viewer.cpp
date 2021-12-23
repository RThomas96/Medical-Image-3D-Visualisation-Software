#include "../include/volumetric_grid_viewer.hpp"

#include "../include/scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

GridViewer::GridViewer(Image::Grid::Ptr& _grid_to_show) {
	// For now, try to display the whole grid.
	// TODO: Downsample the grid given in argument here.

	// By default, the grid is hidden because the data is not yet initialized.
	this->is_grid_hidden	 = true;
	this->grid_texture		 = GL_INVALID_INDEX;
	this->ubo_handle		 = GL_INVALID_INDEX;
	this->texture_handles	 = VolMesh{};
	this->main_channel_index = 0;
	this->source_grid		 = _grid_to_show;
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

void GridViewer::initializeGLData(Scene* _scene) {
	emit this->beforeGLInit();

	// Get grid dimensions :
	glm::tvec4<std::size_t> dimensions{this->source_grid->getResolution(), this->source_grid->getVoxelDimensionality()};
    std::cout << "DIMENSIONS:" << dimensions << std::endl;

	// Upload the texture to the GPU :
	TextureUpload _gridTex{};
	_gridTex.minmag.x  = GL_NEAREST;
	_gridTex.minmag.y  = GL_NEAREST;
	_gridTex.lod.y	   = -1000.f;
	_gridTex.wrap.x	   = GL_CLAMP_TO_EDGE;
	_gridTex.wrap.y	   = GL_CLAMP_TO_EDGE;
	_gridTex.wrap.z	   = GL_CLAMP_TO_EDGE;
	_gridTex.swizzle.r = GL_RED;
	if (dimensions.a > 1) {
		_gridTex.swizzle.g = GL_GREEN;
	} else {
		_gridTex.swizzle.g = GL_ZERO;
	}
	if (dimensions.a > 2) {
		_gridTex.swizzle.b = GL_BLUE;
	} else {
		_gridTex.swizzle.b = GL_ZERO;
	}
	if (dimensions.a > 3) {
		_gridTex.swizzle.a = GL_ALPHA;
	} else {
		_gridTex.swizzle.a = GL_ONE;
	}
	_gridTex.alignment.x = 1;
	_gridTex.alignment.y = 2;
	switch (dimensions.a) {
		case 1:
			_gridTex.format			= GL_RED_INTEGER;
			_gridTex.internalFormat = GL_R16UI;
			break;
		case 2:
			_gridTex.format			= GL_RG_INTEGER;
			_gridTex.internalFormat = GL_RG16UI;
			break;
		case 3:
			_gridTex.format			= GL_RGB_INTEGER;
			_gridTex.internalFormat = GL_RGB16UI;
			break;
		case 4:
			_gridTex.format			= GL_RGBA_INTEGER;
			_gridTex.internalFormat = GL_RGBA16UI;
			break;
	}
	_gridTex.type	= GL_UNSIGNED_SHORT;
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
	this->ubo_handle		= _scene->createUniformBuffer(4 * sizeof(colorChannelAttributes_GL), GL_STATIC_DRAW);
	std::size_t color_s		= sizeof(colorChannelAttributes);
	auto& main_channel_data = this->colorChannelAttributes[this->main_channel_index];
	// Upload data : main channel, then red, green, and blue all at once
	_scene->setUniformBufferData(this->ubo_handle, 0 * color_s, color_s, &main_channel_data);
	// offset by one sizeof(colorChannelAttributes_GL) and upload 3 times that amount at once
	_scene->setUniformBufferData(this->ubo_handle, color_s, 3 * color_s, this->colorChannelAttributes.data());

	// Upload the volumetric mesh's data

	emit this->afterGLInit();
}

void GridViewer::draw3D(Scene* _scene_functions, GLfloat* view_matrix, GLfloat* projection_matrix, glm::vec3 cam_pos) {
	if (this->vis_mode == VisualizationMode::Solid) {
		this->program_handle = _scene_functions->getSolidProgram();
		this->draw_solid(_scene_functions, view_matrix, projection_matrix);
	} else if (this->vis_mode == VisualizationMode::Volumetric || this->vis_mode == VisualizationMode::VolumetricBoxed) {
		this->program_handle = _scene_functions->getVolumetricProgram();
		this->draw_volumetric(_scene_functions, view_matrix, projection_matrix, cam_pos);
	} else {
		std::cerr << "Error : no drawing mode defined for grid " << this->source_grid->getImageName() << '\n';
	}
}

void GridViewer::draw_solid(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix) {
	// Get the program handle to draw and use it :
	_scene->glUseProgram(this->program_handle);

	// Bind the necessary uniforms and the UBO :
	this->bindUniforms_Solid(_scene, view_matrix, projection_matrix);
	this->bindTextures_Solid(_scene);
	this->bindUniformBuffer(_scene, "ColorBlock");

	// Bind the VAO and draw it !
}

void GridViewer::draw_volumetric(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix, glm::vec3 cam_pos) {
	// Get the program handle to draw and use it :
	_scene->glUseProgram(this->program_handle);

	// Bind the necessary uniforms and the UBO :
	this->bindUniforms_Volumetric(_scene, view_matrix, projection_matrix, cam_pos);
	this->bindTextures_Volumetric(_scene);
	this->bindUniformBuffer(_scene, "ColorBlock");

	// Bind the VAO and draw it !
}

void GridViewer::bindUniformBuffer(Scene* _scene, const char* uniform_buffer_name) {
	// find the block index for the current program :
	GLuint block_index = _scene->glGetUniformBlockIndex(this->program_handle, uniform_buffer_name);
	// bind it to bind point 0 (no other UBO for now, it's fine)
	_scene->glUniformBlockBinding(this->program_handle, block_index, 0);
	// bind the buffer to the same point :) :
	_scene->glBindBufferBase(GL_UNIFORM_BUFFER, 0, this->ubo_handle);
}

void GridViewer::bindUniforms_Solid(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix) {
	GLint mMatrix_Loc			   = _scene->findUniform(this->program_handle, "mMatrix");
	GLint vMatrix_Loc			   = _scene->findUniform(this->program_handle, "vMatrix");
	GLint pMatrix_Loc			   = _scene->findUniform(this->program_handle, "pMatrix");
	GLint lightPos_Loc			   = _scene->findUniform(this->program_handle, "lightPos");
	GLint voxelGridOrigin_Loc	   = _scene->findUniform(this->program_handle, "voxelGridOrigin");
	GLint voxelGridSize_Loc		   = _scene->findUniform(this->program_handle, "voxelGridSize");
	GLint voxelSize_Loc			   = _scene->findUniform(this->program_handle, "voxelSize");
	GLint drawMode_Loc			   = _scene->findUniform(this->program_handle, "drawMode");
	GLint planePositionsLoc		   = _scene->findUniform(this->program_handle, "planePositions");
	GLint location_planeDirections = _scene->findUniform(this->program_handle, "planeDirections");
	GLint gridPositionLoc		   = _scene->findUniform(this->program_handle, "gridPosition");

	// Get bounding box details :
	DiscreteGrid::bbox_t::vec origin   = this->source_grid->getBoundingBox().getMin();
	DiscreteGrid::bbox_t::vec originWS = this->source_grid->getBoundingBox().getMin();
	DiscreteGrid::sizevec3 gridDims	   = this->source_grid->getResolution();
	glm::vec3 dims					   = glm::convert_to<float>(gridDims);

	// get plane and light position, and plane direction :
	glm::vec3 planePos = _scene->computePlanePositions();
	glm::vec3 planeDir = _scene->getPlaneDirections();
	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	TransformStack::Ptr stack	 = this->source_grid->getTransformStack();
	MatrixTransform::Ptr transfo = stack->getPrecomputedMatrix();

	_scene->glUniform3fv(voxelGridOrigin_Loc, 1, glm::value_ptr(origin));
	_scene->glUniform3fv(voxelGridSize_Loc, 1, glm::value_ptr(dims));
	_scene->glUniform3fv(voxelSize_Loc, 1, glm::value_ptr(this->source_grid->getVoxelDimensions()));
	// Below seems to be useless. To check.
	_scene->glUniform1ui(drawMode_Loc, this->vis_mode);

	_scene->glUniform3fv(planePositionsLoc, 1, glm::value_ptr(planePos));
	_scene->glUniform3fv(location_planeDirections, 1, glm::value_ptr(planeDir));
	_scene->glUniform3fv(gridPositionLoc, 1, glm::value_ptr(originWS));

	// Apply the uniforms :
	_scene->glUniformMatrix4fv(mMatrix_Loc, 1, GL_FALSE, glm::value_ptr(transfo->matrix()));
	_scene->glUniformMatrix4fv(vMatrix_Loc, 1, GL_FALSE, &view_matrix[0]);
	_scene->glUniformMatrix4fv(pMatrix_Loc, 1, GL_FALSE, &projection_matrix[0]);
	_scene->glUniform4fv(lightPos_Loc, 1, glm::value_ptr(lightPos));
}

void GridViewer::bindTextures_Solid(Scene* _scene) {
	// Grid textures :
	GLint texDataLoc = _scene->findUniform(this->program_handle, "texData");
	// The color scales :
	GLint location_colorScales0 = _scene->findUniform(this->program_handle, "colorScales[0]");
	GLint location_colorScales1 = _scene->findUniform(this->program_handle, "colorScales[1]");
	GLint location_colorScales2 = _scene->findUniform(this->program_handle, "colorScales[2]");
	GLint location_colorScales3 = _scene->findUniform(this->program_handle, "colorScales[3]");

	glm::tvec4<GLuint> colorscales = _scene->draft_getGeneratedColorScales();

	GLuint enabled_textures = 0;

	_scene->glEnable(GL_TEXTURE_3D);
	// Grid texture :
	_scene->glActiveTexture(GL_TEXTURE0 + enabled_textures);
	_scene->glBindTexture(GL_TEXTURE_3D, this->grid_texture);
	_scene->glUniform1i(texDataLoc, enabled_textures);
	enabled_textures++;

	_scene->glEnable(GL_TEXTURE_1D);
	// Color scales :
	_scene->glActiveTexture(GL_TEXTURE0 + enabled_textures);
	_scene->glBindTexture(GL_TEXTURE_1D, colorscales.x);
	_scene->glUniform1i(location_colorScales0, enabled_textures);
	enabled_textures++;
	_scene->glActiveTexture(GL_TEXTURE0 + enabled_textures);
	_scene->glBindTexture(GL_TEXTURE_1D, colorscales.y);
	_scene->glUniform1i(location_colorScales1, enabled_textures);
	enabled_textures++;
	_scene->glActiveTexture(GL_TEXTURE0 + enabled_textures);
	_scene->glBindTexture(GL_TEXTURE_1D, colorscales.z);
	_scene->glUniform1i(location_colorScales2, enabled_textures);
	enabled_textures++;
	_scene->glActiveTexture(GL_TEXTURE0 + enabled_textures);
	_scene->glBindTexture(GL_TEXTURE_1D, colorscales.w);
	_scene->glUniform1i(location_colorScales3, enabled_textures);
	enabled_textures++;
}

void GridViewer::bindUniforms_Volumetric(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix, glm::vec3 cam_pos) {
	// Vectors/arrays :
	GLint location_voxelSize			  = _scene->findUniform(this->program_handle, "voxelSize");
	GLint location_gridSize				  = _scene->findUniform(this->program_handle, "gridSize");
	GLint location_cam					  = _scene->findUniform(this->program_handle, "cam");
	GLint location_cut					  = _scene->findUniform(this->program_handle, "cut");
	GLint location_cutDirection			  = _scene->findUniform(this->program_handle, "cutDirection");
	GLint location_visuBBMin			  = _scene->findUniform(this->program_handle, "visuBBMin");
	GLint location_visuBBMax			  = _scene->findUniform(this->program_handle, "visuBBMax");
	GLint location_shouldUseBB			  = _scene->findUniform(this->program_handle, "shouldUseBB");
	GLint location_volumeEpsilon		  = _scene->findUniform(this->program_handle, "volumeEpsilon");
	GLint location_clipDistanceFromCamera = _scene->findUniform(this->program_handle, "clipDistanceFromCamera");

	glm::vec3 floatres			  = glm::convert_to<float>(this->source_grid->getResolution());
	glm::vec3 planePos			  = _scene->computePlanePositions();
	glm::vec3 planeDir			  = _scene->getPlaneDirections();
	DiscreteGrid::bbox_t box	  = _scene->getVisuBox();
	DiscreteGrid::bbox_t::vec min = box.getMin();
	DiscreteGrid::bbox_t::vec max = box.getMax();

	_scene->glUniform3fv(location_voxelSize, 1, glm::value_ptr(this->source_grid->getVoxelDimensions()));
	_scene->glUniform3fv(location_gridSize, 1, glm::value_ptr(floatres));
	_scene->glUniform3fv(location_cam, 1, glm::value_ptr(cam_pos));
	_scene->glUniform3fv(location_cut, 1, glm::value_ptr(planePos));
	_scene->glUniform3fv(location_cutDirection, 1, glm::value_ptr(planeDir));
	_scene->glUniform3fv(location_visuBBMin, 1, glm::value_ptr(min));
	_scene->glUniform3fv(location_visuBBMax, 1, glm::value_ptr(max));
	_scene->glUniform1ui(location_shouldUseBB, ((this->vis_mode == VisualizationMode::VolumetricBoxed) ? 1 : 0));
	_scene->glUniform3fv(location_volumeEpsilon, 1, glm::value_ptr(this->volumetric_epsilon));
	_scene->glUniform1f(location_clipDistanceFromCamera, 5.f);
	// TODO: Argument above should be clipDistanceFromCamera, and should be user-defined (or at least user-modifiable)

	// Matrices :
	GLint location_mMat = _scene->findUniform(this->program_handle, "mMat");
	GLint location_vMat = _scene->findUniform(this->program_handle, "vMat");
	GLint location_pMat = _scene->findUniform(this->program_handle, "pMat");

	MatrixTransform::Ptr grid_transform_pointer = this->source_grid->getTransformStack()->getPrecomputedMatrix();
	const glm::mat4 gridTransfo					= grid_transform_pointer->matrix();

	_scene->glUniformMatrix4fv(location_mMat, 1, GL_FALSE, glm::value_ptr(gridTransfo));
	_scene->glUniformMatrix4fv(location_vMat, 1, GL_FALSE, &view_matrix[0]);
	_scene->glUniformMatrix4fv(location_pMat, 1, GL_FALSE, &projection_matrix[0]);

	// Color and shading parameters :
	GLint location_specRef	  = _scene->findUniform(this->program_handle, "specRef");
	GLint location_shininess  = _scene->findUniform(this->program_handle, "shininess");
	GLint location_diffuseRef = _scene->findUniform(this->program_handle, "diffuseRef");

	_scene->glUniform1f(location_specRef, .8f);
	_scene->glUniform1f(location_shininess, .8f);
	_scene->glUniform1f(location_diffuseRef, .8f);

	// Light positions :
	auto lightPositions	  = _scene->getLights();
	GLint location_light0 = _scene->findUniform(this->program_handle, "lightPositions[0]");
	GLint location_light1 = _scene->findUniform(this->program_handle, "lightPositions[1]");
	GLint location_light2 = _scene->findUniform(this->program_handle, "lightPositions[2]");
	GLint location_light3 = _scene->findUniform(this->program_handle, "lightPositions[3]");
	GLint location_light4 = _scene->findUniform(this->program_handle, "lightPositions[4]");
	GLint location_light5 = _scene->findUniform(this->program_handle, "lightPositions[5]");
	GLint location_light6 = _scene->findUniform(this->program_handle, "lightPositions[6]");
	GLint location_light7 = _scene->findUniform(this->program_handle, "lightPositions[7]");

	_scene->glUniform3fv(location_light0, 1, glm::value_ptr(lightPositions[0]));
	_scene->glUniform3fv(location_light1, 1, glm::value_ptr(lightPositions[1]));
	_scene->glUniform3fv(location_light2, 1, glm::value_ptr(lightPositions[2]));
	_scene->glUniform3fv(location_light3, 1, glm::value_ptr(lightPositions[3]));
	_scene->glUniform3fv(location_light4, 1, glm::value_ptr(lightPositions[4]));
	_scene->glUniform3fv(location_light5, 1, glm::value_ptr(lightPositions[5]));
	_scene->glUniform3fv(location_light6, 1, glm::value_ptr(lightPositions[6]));
	_scene->glUniform3fv(location_light7, 1, glm::value_ptr(lightPositions[7]));
}

void GridViewer::bindTextures_Volumetric(Scene* _scene) {
	// Get outside-generated color scales
	glm::tvec4<GLuint> textures = _scene->draft_getGeneratedColorScales();

	// Texture handles :
	GLint location_vertices_translation	  = _scene->findUniform(this->program_handle, "vertices_translations");
	GLint location_normals_translation	  = _scene->findUniform(this->program_handle, "normals_translations");
	GLint location_visibility_texture	  = _scene->findUniform(this->program_handle, "visibility_texture");
	GLint location_texture_coordinates	  = _scene->findUniform(this->program_handle, "texture_coordinates");
	GLint location_neighbors			  = _scene->findUniform(this->program_handle, "neighbors");
	GLint location_Mask					  = _scene->findUniform(this->program_handle, "texData");
	GLint location_visibilityMap		  = _scene->findUniform(this->program_handle, "visiblity_map");
	GLint location_visibilityMapAlternate = _scene->findUniform(this->program_handle, "visiblity_map_alternate");

	std::size_t tex = 0;
	_scene->glActiveTexture(GL_TEXTURE0 + tex);
	_scene->glBindTexture(GL_TEXTURE_2D, this->texture_handles.vertexPositions);
	_scene->glUniform1i(location_vertices_translation, tex);
	tex++;

	_scene->glActiveTexture(GL_TEXTURE0 + tex);
	_scene->glBindTexture(GL_TEXTURE_2D, this->texture_handles.faceNormals);
	_scene->glUniform1i(location_normals_translation, tex);
	tex++;

	_scene->glActiveTexture(GL_TEXTURE0 + tex);
	_scene->glBindTexture(GL_TEXTURE_2D, this->texture_handles.visibilityMap);
	_scene->glUniform1i(location_visibility_texture, tex);
	tex++;

	_scene->glActiveTexture(GL_TEXTURE0 + tex);
	_scene->glBindTexture(GL_TEXTURE_2D, this->texture_handles.textureCoordinates);
	_scene->glUniform1i(location_texture_coordinates, tex);
	tex++;

	_scene->glActiveTexture(GL_TEXTURE0 + tex);
	_scene->glBindTexture(GL_TEXTURE_2D, this->texture_handles.neighborhood);
	_scene->glUniform1i(location_neighbors, tex);
	tex++;

	_scene->glActiveTexture(GL_TEXTURE0 + tex);
	_scene->glBindTexture(GL_TEXTURE_3D, this->grid_texture);
	_scene->glUniform1i(location_Mask, tex);
	tex++;

	_scene->glActiveTexture(GL_TEXTURE0 + tex);
#warning Should be called only after the generateAndUploadVisibilityTexture() function is implemented ! \
			Will not see anything otherwise !!!
	_scene->glBindTexture(GL_TEXTURE_2D, this->visibility_texture);
	_scene->glUniform1i(location_visibilityMap, tex);
	tex++;

	GLint location_colorScales0 = _scene->findUniform(this->program_handle, "colorScales[0]");
	GLint location_colorScales1 = _scene->findUniform(this->program_handle, "colorScales[1]");
	GLint location_colorScales2 = _scene->findUniform(this->program_handle, "colorScales[2]");
	GLint location_colorScales3 = _scene->findUniform(this->program_handle, "colorScales[3]");

	_scene->glActiveTexture(GL_TEXTURE0 + tex);
	_scene->glBindTexture(GL_TEXTURE_1D, textures.x);
	_scene->glUniform1i(location_colorScales0, tex);
	tex++;

	_scene->glActiveTexture(GL_TEXTURE0 + tex);
	_scene->glBindTexture(GL_TEXTURE_1D, textures.y);
	_scene->glUniform1i(location_colorScales1, tex);
	tex++;

	_scene->glActiveTexture(GL_TEXTURE0 + tex);
	_scene->glBindTexture(GL_TEXTURE_1D, textures.z);
	_scene->glUniform1i(location_colorScales2, tex);
	tex++;

	_scene->glActiveTexture(GL_TEXTURE0 + tex);
	_scene->glBindTexture(GL_TEXTURE_1D, textures.w);
	_scene->glUniform1i(location_colorScales3, tex);
	tex++;
}

void GridViewer::updateMainChannel_UBO(Scene* _scene) {
	// If the data was not already set and created, do nothing
	if (this->ubo_handle == GL_INVALID_INDEX) {
		return;
	}

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
	const vec_t min	 = this->source_grid->getBoundingBox().getMin();
	const vec_t diag = this->source_grid->getBoundingBox().getDiagonal();
	// Dimensions, subject to change :
	std::size_t xv			 = 10;
	glm::vec4::value_type xs = diag.x / static_cast<glm::vec4::value_type>(xv);
	std::size_t yv			 = 10;
	glm::vec4::value_type ys = diag.y / static_cast<glm::vec4::value_type>(yv);
	std::size_t zv			 = 10;
	glm::vec4::value_type zs = diag.z / static_cast<glm::vec4::value_type>(zv);

	// Size of tetrahedra, to compute epsilon :
	glm::vec3 epsVolu		 = glm::vec3(xs, ys, zs);
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
				pos = transfo * glm::vec4(min.x + static_cast<val_t>(i) * xs, min.y + static_cast<val_t>(j) * ys,
								  min.z + static_cast<val_t>(k) * zs, 1.f);
				//
				tex = glm::vec3(
				  static_cast<val_t>(i) / static_cast<val_t>(xv),
				  static_cast<val_t>(j) / static_cast<val_t>(yv),
				  static_cast<val_t>(k) / static_cast<val_t>(zv));

				this->volumetric_mesh.positions.push_back(pos);
				this->volumetric_mesh.texture.push_back(tex);
			}
		}
	}

	// Return position for vertex generated at indices I, J, K :
	std::size_t xt = xv + 1;
	std::size_t yt = yv + 1;
	auto getIndice = [&, xt, yt](std::size_t i, std::size_t j, std::size_t k) -> std::size_t {
		return i + j * xt + k * xt * yt;
	};

	// Create tetrahedra :
	for (std::size_t k = 0; k < zv; ++k) {
		for (std::size_t j = 0; j < yv; ++j) {
			for (std::size_t i = 0; i < xv; ++i) {
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i + 1, j, k), getIndice(i + 1, j + 1, k), getIndice(i, j + 1, k), getIndice(i + 1, j + 1, k + 1)});
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i, j, k + 1), getIndice(i, j, k), getIndice(i, j + 1, k + 1), getIndice(i + 1, j, k + 1)});
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i, j + 1, k + 1), getIndice(i + 1, j, k), getIndice(i + 1, j + 1, k + 1), getIndice(i + 1, j, k + 1)});
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i, j, k), getIndice(i + 1, j, k), getIndice(i, j + 1, k + 1), getIndice(i + 1, j, k + 1)});
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i, j, k), getIndice(i + 1, j, k), getIndice(i, j + 1, k), getIndice(i, j + 1, k + 1)});
				this->volumetric_mesh.tetrahedra.push_back({getIndice(i, j + 1, k), getIndice(i + 1, j, k), getIndice(i + 1, j + 1, k + 1), getIndice(i, j + 1, k + 1)});
			}
		}
	}

	/* Mesh is now built, and should conform to the grid well enough.
	 * Some artefacts might appear at some orientations, due to floating
	 * point imprecision in the DDA tracing. They will be along the edge
	 * of the mesh's tetrahedra. */

	return;
}

void GridViewer::generateAndUploadVisibilityTexture(Scene* _scene) {
#warning For now, must pass the Scene in argument but should be done differently.
	std::size_t vox_dim = this->source_grid->getVoxelDimensionality();
	float* visibility	= new float[std::size_t(256ul) * std::size_t(256ul) * vox_dim];

	using bound_t = colorChannelAttributes_GL::bound_t;

	std::array<bound_t, 3> bounds = {
	  this->colorChannelAttributes[0].getVisibleRange(),
	  this->colorChannelAttributes[1].getVisibleRange(),
	  this->colorChannelAttributes[2].getVisibleRange()};

	// for all values in the visible range :
	for (std::size_t i = 0; i < 65536ul; ++i) {
		// for all channels :
		for (std::size_t v = 0; v < vox_dim; ++v) {
			// is the value 'i' visible on channel 'v' ?
			bool visible = (i >= bounds[v].x && i <= bounds[v].y);
			// act accordingly :
			visibility[i * vox_dim + v] = visible ? 1.f : 0.f;
		}
	}

	TextureUpload texParams;
	// create texture upload data :
	texParams.minmag.x	= GL_NEAREST;
	texParams.minmag.y	= GL_NEAREST;
	texParams.lod.y		= -1000.f;
	texParams.wrap.s	= GL_CLAMP_TO_EDGE;
	texParams.wrap.t	= GL_CLAMP_TO_EDGE;
	texParams.swizzle.x = GL_RED;	 // At least one channel visible.
	// if more, 'enable' more channels (don't swizzle them)
	if (vox_dim > 1) {
		texParams.swizzle.y = GL_GREEN;
	} else {
		texParams.swizzle.y = GL_ZERO;
	}
	if (vox_dim > 2) {
		texParams.swizzle.z = GL_BLUE;
	} else {
		texParams.swizzle.z = GL_ZERO;
	}
	// alpha is always one, we're doing RGB channels only here :
	texParams.swizzle.a = GL_ONE;

	// Set the right internal format and pixel format :
	if (vox_dim == 1) {
		texParams.internalFormat = GL_R32F;
		texParams.format		 = GL_RED;
	} else if (vox_dim == 2) {
		texParams.internalFormat = GL_RG32F;
		texParams.format		 = GL_RG;
	} else if (vox_dim == 3) {
		texParams.internalFormat = GL_RGB32F;
		texParams.format		 = GL_RGB;
	}
	// For now, size'll always be 65536 (so 256 squared) :
	texParams.size.x = 256;
	texParams.size.y = 256;
	texParams.type	 = GL_FLOAT;
	// The data generated beforehand :
	texParams.data			 = visibility;
	this->visibility_texture = _scene->uploadTexture2D(texParams);
}

void GridViewer::hide(bool _should_hide) {
	this->is_grid_hidden = _should_hide;
	emit this->visibilityChanged(_should_hide);
}

void GridViewer::setViewMode(GridViewer::VisualizationMode _new_mode) {
	this->vis_mode = _new_mode;
	emit this->viewModeChanged(_new_mode);
}

void GridViewer::setMainChannel(std::uint8_t _new_main_channel) {
	if (_new_main_channel >= 3) {
		return;
	}
	this->main_channel_index = _new_main_channel;
	emit this->mainChannelChanged(_new_main_channel);
}
