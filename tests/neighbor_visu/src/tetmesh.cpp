#include "../include/tetmesh.hpp"

TetMesh::TetMesh(const std::shared_ptr<TextureStorage> texL) : texLoader(texL) {
	// Initialises all the values of the mesh to their default values.
	// Centers the mesh around the space's origin. Also, makes the mesh
	// by a call to TetMesh::makeTetrahedra().
	this->vertices.clear();
	this->vertexValues.clear();
	this->tetrahedra.clear();
	this->origin = glm::vec4(.0, .0, .0, .0);

	this->makeTetrahedra();
}

TetMesh::~TetMesh() {
	// Free all storage allocated for the current mesh.
	this->vertices.clear();
	this->vertexValues.clear();
	this->tetrahedra.clear();
}

TetMesh& TetMesh::setOrigin(const glm::vec4 position) {
	// If no stack manager is linked, there exists no conversion from real space to intial
	// space. So nothing happens, and an error message is displayed to the user.
	if (this->texLoader == nullptr) {
		std::cerr << "Warning : asked a world space position, but no stack was"
			  << " linked with this mesh !\n";
		return *this;
	}
	glm::vec4 posInitialSpace = this->texLoader->convertRealSpaceToInitialSpace(position);
	return this->setOriginInitialSpace(posInitialSpace);
}

TetMesh& TetMesh::setOriginInitialSpace(const glm::vec4 position) {
	// The position given in argument is a 'free' position,
	// not constrained to the grid's voxel centers. We need
	// to set the center of the mesh to the center of the
	// nearest voxel :
	glm::vec4 newOrigin = glm::vec4(std::truncf(position.x), std::truncf(position.y), std::truncf(position.z), .0f);

	// Set the new origin to be the center of the nearest
	// voxel relative to 'position'
	this->updatePositions(newOrigin);

	return *this;
}

TetMesh& TetMesh::resetPositions() {
	// Resets the positions so the mesh is centered around the origin.
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		this->vertices[i] -= this->origin;
		this->vertices[i].w = 1.; // Sanity check.
	}
	return *this;
}

TetMesh& TetMesh::updateValues() {
	// Get the value of the voxel each vertex is positionned in. Nearest neighbor only on the positions.
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		const glm::vec4& icoord = this->vertices[i];
		#ifndef NDEBUG // Debug only prompt
		std::cerr << "Update :: querying for (" << icoord.x << ',' << icoord.y << ',' << icoord.z << ")\n";
		#endif
		this->vertexValues[i] = this->texLoader->getTexelValue(icoord);
	}
	return *this;
}

TetMesh& TetMesh::updatePositions(glm::vec4 newOrigin) {
	// Centers the mesh's vertices around the new origin given, by first
	// centering them around the space's origin and them translating them
	// by newOrigin. Then updates the mesh's values at those vertices.

	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		this->vertices[i] = this->vertices[i] - this->origin + newOrigin;
		this->vertices[i].w = 1.; // Sanity check.
	}
	this->updateValues();
	this->origin = newOrigin;

	return *this;
}

std::vector<glm::vec4> TetMesh::getVertices() const {
	// Simple getter for the mesh's vertices.
	std::vector<glm::vec4> res(this->vertices);
	return res;
}

std::vector<unsigned char> TetMesh::getVertexValues() const {
	// Simple getter for the mesh's vertex values.
	return this->vertexValues;
}

TetMesh& TetMesh::printInfo() {
	// Prints info about the mesh :
	//   - the image size,
	//   - the minimum value of the image's data bounding box,
	//   - the maximum value of the image's data bounding box,
	//   - the vertices positions and values at the current time.

	std::cout << "The image storage has the following specs :" << '\n';
	std::vector<svec3> specs = this->texLoader->getImageSpecs();
	std::cout << "Image size : " << specs[0][0] << 'x' << specs[0][1] << 'x' << specs[0][2] << '\n';
	std::cout << "Bounding box min value: " << specs[1][0] << 'x' << specs[1][1] << 'x' << specs[1][2] << '\n';
	std::cout << "Bounding box max value: " << specs[2][0] << 'x' << specs[2][1] << 'x' << specs[2][2] << '\n';

	std::cout << "The neighboring mesh has the following positions and values :" << '\n';
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		std::cout << "[" << this->vertices[i].x << ',' << this->vertices[i].y << ',' << this->vertices[i].z << "] : " << +(this->vertexValues[i]) << '\n';
	}

	return *this;
}

unsigned char TetMesh::getInterpolatedValue(glm::vec4 pos_ws, InterpolationMethods method) {
	// If no stack loader was linked with the current mesh, then we cannot infer the data
	// that should be at a given point [XYZ] in either real or intial space. Return 0 in
	// this case. Otherwise, return the value at this point, converted in initial space.
	if (this->texLoader == nullptr) {
		std::cerr << "Warning : asked a world space position, but no stack was"
			  << " linked with this mesh !\n";
		return 0;
	}
	glm::vec4 posInitialSpace = this->texLoader->convertRealSpaceToInitialSpace(pos_ws);
	return this->getInterpolatedValueInitialSpace(posInitialSpace, method);
}

unsigned char TetMesh::getInterpolatedValueInitialSpace(glm::vec4 pos_is, InterpolationMethods method) {
	// Same deal as getInterpolatedValue(), but in initial space. Then, calls the appropriate method
	// to get an interpolated value at the specified point.
	// If the method is not recognized, then the function returns 0 (no data).

	if (this->texLoader == nullptr) {
		std::cerr << "Warning : asked to interpolate a point in the mesh, but no image stack was"
			  << " linked with it !\n";
		return 0;
	}

	switch (method) {
		case InterpolationMethods::NearestNeighbor:
			return this->interpolate_NearestNeighbor(pos_is);
		case InterpolationMethods::TriLinear:
			return this->interpolate_TriLinear(pos_is);
		case InterpolationMethods::TriCubic:
			return this->interpolate_TriCubic(pos_is);
		case InterpolationMethods::Barycentric:
			return this->interpolate_Barycentric(pos_is);
		default:
			std::cerr << "Method asked for interpolation was not recognised.\n";
			return 0;
	}
}

unsigned char TetMesh::interpolate_NearestNeighbor(glm::vec4 pos) const {
	// TexStorage::getTexelValue() already applies NearestNeighbor on the position
	// given in argument, so we just fetch the value of the function :
	return this->texLoader->getTexelValue(pos);
}

unsigned char TetMesh::interpolate_TriLinear(glm::vec4 pos) const {
	// Get the vertices nearest the point asked for. Then, use the point's
	// X, Y, and Z coordinates to linearly sample the value requested.

	// Determine which 'sub-cube' the point belongs to :
	std::size_t index_x_min = (pos.x < this->origin.x) ? 0 : 1;
	std::size_t index_y_min = (pos.y < this->origin.y) ? 0 : 1;
	std::size_t index_z_min = (pos.z < this->origin.z) ? 0 : 1;
	std::size_t index_x_max = index_x_min + 1;
	std::size_t index_y_max = index_y_min + 1;
	std::size_t index_z_max = index_z_min + 1;

	// Since the voxels are of size 1 in initial space, we only need the coordinates
	// of the vertex closest to the origin within the ones where the point is located.
	pos -= this->vertices[index_x_min * 2*2 + index_y_min * 2 + index_z_min];
	// This gets the cube the point is in to the origin (so we can interpolate directly
	// with its coordinates)

	// Get values :
	const uchar& xyz = this->vertexValues[index_x_min * 2*2 + index_y_min * 2 + index_z_min];
	const uchar& xyZ = this->vertexValues[index_x_min * 2*2 + index_y_min * 2 + index_z_max];
	const uchar& xYz = this->vertexValues[index_x_min * 2*2 + index_y_max * 2 + index_z_min];
	const uchar& xYZ = this->vertexValues[index_x_min * 2*2 + index_y_max * 2 + index_z_max];
	const uchar& Xyz = this->vertexValues[index_x_max * 2*2 + index_y_min * 2 + index_z_min];
	const uchar& XyZ = this->vertexValues[index_x_max * 2*2 + index_y_min * 2 + index_z_max];
	const uchar& XYz = this->vertexValues[index_x_max * 2*2 + index_y_max * 2 + index_z_min];
	const uchar& XYZ = this->vertexValues[index_x_max * 2*2 + index_y_max * 2 + index_z_max];

	float cyz = (1.f - pos.x) * static_cast<float>(xyz) + pos.x * static_cast<float>(Xyz);
	float cyZ = (1.f - pos.x) * static_cast<float>(xyZ) + pos.x * static_cast<float>(XyZ);
	float cYz = (1.f - pos.x) * static_cast<float>(xYz) + pos.x * static_cast<float>(XYz);
	float cYZ = (1.f - pos.x) * static_cast<float>(xYZ) + pos.x * static_cast<float>(XYZ);

	float cz = (1.f - pos.y) * cyz + pos.y * cYz;
	float cZ = (1.f - pos.y) * cyZ + pos.y * cYZ;

	return static_cast<unsigned char>((1.f - pos.z) * cz + pos.z * cZ);
}

unsigned char TetMesh::interpolate_TriCubic(glm::vec4 pos) const {
	// Not implemented : need the neighbors to a distance of 2, not 1 as it is now
	std::cerr << "Warning : the method is not yer implemented !\n";
	return 0;
}

unsigned char TetMesh::interpolate_Barycentric(glm::vec4 pos) const {
	// Get the tetrahedron in which the point resides. Then, compute its barycentric
	// coordinates and interpolate the values of the tetrahedron's vertices with the
	// coefficients of the vertex in it.

	bool found = false;
	std::size_t i = 0;
	for (i; i < this->tetrahedra.size(); ++i) {
		found = isPointInTetrahedra(pos, i);
		if (found) { break; }
	}
	if (not found) {
		// Not in any tetrahedra ... strange.
		std::cerr << "Warning : the point was not found to be in any tetrahedra.\n";
		return 0;
	}

	// TODO : compute barycentric coordinates for tetrahedron 'i', and then interpolate the values.
}

bool TetMesh::isPointInTetrahedra(const glm::vec4 pos, std::size_t tet) const {
	// Computes the planes corresponding to the faces of the tetrahedra, with normals
	// oriented inwards. If the point is on the 'good' side of each plane, then it
	// **must** be in the tetrahedra.

	// Note : glm doesn't appear to have a functionnal cast from glm::vec3 to glm::vec4,
	// so we are forced to allocate more on the stack than necessary and jump through
	// some hoops in order to do this :

	const glm::vec4 t0 = this->vertices[this->tetrahedra[tet][0]]; glm::vec3 p0 = glm::vec3(t0.x, t0.y, t0.z);
	const glm::vec4 t1 = this->vertices[this->tetrahedra[tet][1]]; glm::vec3 p1 = glm::vec3(t1.x, t1.y, t1.z);
	const glm::vec4 t2 = this->vertices[this->tetrahedra[tet][2]]; glm::vec3 p2 = glm::vec3(t2.x, t2.y, t2.z);
	const glm::vec4 t3 = this->vertices[this->tetrahedra[tet][3]]; glm::vec3 p3 = glm::vec3(t3.x, t3.y, t3.z);

	glm::vec3 n0 = glm::cross((p1 - p0), (p2 - p0)); n0 = (glm::dot(n0, p3) < 0) ? -n0 : n0; glm::vec4 N0 = glm::vec4(n0.x, n0.y, n0.z, .0f);
	glm::vec3 n1 = glm::cross((p1 - p0), (p3 - p0)); n1 = (glm::dot(n1, p2) < 0) ? -n1 : n1; glm::vec4 N1 = glm::vec4(n1.x, n1.y, n1.z, .0f);
	glm::vec3 n2 = glm::cross((p2 - p0), (p3 - p0)); n2 = (glm::dot(n2, p1) < 0) ? -n2 : n2; glm::vec4 N2 = glm::vec4(n2.x, n2.y, n2.z, .0f);
	glm::vec3 n3 = glm::cross((p2 - p1), (p3 - p1)); n3 = (glm::dot(n3, p0) < 0) ? -n3 : n3; glm::vec4 N3 = glm::vec4(n3.x, n3.y, n3.z, .0f);

	return (glm::dot(pos, N0) > .0f && glm::dot(pos, N1) > .0f && glm::dot(pos, N2) > .0f && glm::dot(pos, N3) > .0f);
}

void TetMesh::makeTetrahedra() {
	// For now, a mesh of side 1, centered at the origin

	auto getIndex = [&](std::size_t i, std::size_t j, std::size_t k) {
		return i * 2u * 2u + j * 2u + k;
	};

	// At the start, the origin is at [.0, .0, .0]. But the center of
	// the first voxel is in fact at [.5, .5, .5], which means we
	// need to iterate in [-.5, 1.5] by steps of 1. each time.

	for (std::size_t i = 0; i < 3; ++i) {
		for (std::size_t j = 0; j < 3; ++j) {
			for (std::size_t k = 0; k < 3; ++k) {
				this->vertices.emplace_back(
					static_cast<float>(i) * 1.f - .5f,
					static_cast<float>(j) * 1.f - .5f,
					static_cast<float>(k) * 1.f - .5f,
					1.
				);
				this->vertexValues.push_back(0);
			}
		}
	}

	for (std::size_t i = 0; i < 2; ++i) {
		for (std::size_t j = 0; j < 2; ++j) {
			for (std::size_t k = 0; k < 2; ++k) {
				// Tetrahedra 1 :
				this->tetrahedra.push_back({
					getIndex(i+1, j+0, k+0),
					getIndex(i+1, j+1, k+0),
					getIndex(i+0, j+1, k+0),
					getIndex(i+1, j+1, k+1)
				});
				// Tetrahedra 2 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+0, k+1),
					getIndex(i+0, j+0, k+0),
					getIndex(i+0, j+1, k+1),
					getIndex(i+1, j+0, k+1)
				});
				// Tetrahedra 3 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+1, k+1),
					getIndex(i+1, j+0, k+0),
					getIndex(i+1, j+1, k+1),
					getIndex(i+1, j+0, k+1)
				});
				// Tetrahedra 4 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+0, k+0),
					getIndex(i+1, j+0, k+0),
					getIndex(i+0, j+1, k+1),
					getIndex(i+1, j+0, k+1)
				});
				// Tetrahedra 5 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+0, k+0),
					getIndex(i+1, j+0, k+0),
					getIndex(i+0, j+1, k+0),
					getIndex(i+0, j+1, k+1)
				});
				// Tetrahedra 6 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+1, k+0),
					getIndex(i+1, j+0, k+0),
					getIndex(i+1, j+1, k+1),
					getIndex(i+0, j+1, k+1)
				});
			}
		}
	}

	// The mesh is now constructed. Or at least, it should be.
}






















































