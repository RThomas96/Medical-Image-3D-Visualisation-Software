#ifndef ASRIGIDASPOSSIBLE_H
#define ASRIGIDASPOSSIBLE_H

#include "../../base_mesh/Triangle.h"
#include "Edge.h"
#include "./Manipulator.h"

#include <glm/glm.hpp>
#include <gsl/gsl_linalg.h>
#include <cholmod.h>

#include <iostream>

class AsRigidAsPossible {
public:
	AsRigidAsPossible();

	~AsRigidAsPossible();
	/// @brief Computes the one-ring, defines handles for the instance, and computes cotangeant weights for the mesh.
	void init(const std::vector<glm::vec3> &_vertices, const std::vector<Triangle> &_triangles);
	/// @brief Computes the one-ring, defines handles for the instance, and computes cotangeant weights for the mesh.
	void init(const std::vector<glm::vec3> &_vertices, const std::vector<std::vector<int>> &_triangles);

	void setVertices(const std::vector<glm::vec3> &_vertices);

	inline std::vector<glm::vec3> &getVertices() { return vertices; }
	inline const std::vector<glm::vec3> getVertices() const { return vertices; }

	inline std::vector<bool> &getHandles() { return handles; }
	inline const std::vector<bool> &getHandles() const { return handles; }

	void setHandles(const std::vector<bool> &_handles);
	void compute_deformation(std::vector<glm::vec3> &positions);

	void setIterationNb(unsigned int itNb) { iterationNb = itNb; }
	unsigned int getIterationNb() { return iterationNb; }

	void draw();

	std::vector<glm::vec3> dummy_deformation(float threshold_on_x, glm::vec3 displacement, glm::vec3 min, glm::vec3 max);

	void clear();

protected:
	void compute_product_and_sum(gsl_matrix *R, const glm::vec3 &point, glm::vec3 &result);
	void compute_S(gsl_matrix *S, unsigned int vi, const std::vector<glm::vec3> &pdef);
	void compute_R(gsl_matrix *R, gsl_matrix *U, gsl_matrix *V);
	float compute_determinant(gsl_matrix *M);
	void singular_value_decomposition(gsl_matrix *matrix, gsl_matrix *U, gsl_matrix *V);
	void factorize_cholmod_A_system();
	void add_A_coeff(const int row, const int col, const double value);
	void update_A_coeff(const int i, const double value);
	void set_b_value(const int i, const glm::vec3 &value);
	cholmod_dense *solve_cholmod();
	void allocates_cholmod_A_and_b();
	void fill_cholmod_A();
	void setDefaultRotations();

	int constrainedNb;
	// PARTIE CHOLMOD , ininteressante //
	int _rows;
	int _cols;
	cholmod_triplet *_triplet;
	int *_rowPtrA;
	int *_colPtrA;
	double *_valuePtrA;

	cholmod_sparse *_At;

	cholmod_factor *_L;

	cholmod_dense *_b;
	double *_valuePtrB;

	cholmod_common _c;

	int _nb_non_zeros_in_A;

	unsigned int step;

	bool data_loaded;
	/////////////////////////////////////
	unsigned int iterationNb;
	std::vector<glm::vec3> vertices;
	CotangentWeights edgesWeightMap;
	std::map<Edge, glm::vec3, compareEdge> bij;
	std::vector<bool> handles;
	std::vector<std::vector<unsigned int>> oneRing;
	std::vector<gsl_matrix *> R;
	std::vector<float> sumWij;
};

#endif	  // ASRIGIDASPOSSIBLE_H
