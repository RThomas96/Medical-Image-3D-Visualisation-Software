#include "cage_surface_mesh.hpp"
#include "mesh_deformer.hpp"

void toBasicPoint(const std::vector<glm::vec3>& points, std::vector<BasicPoint>& res) {
    res.clear();
    for(int i = 0; i < points.size(); ++i)
        res.push_back(BasicPoint(points[i][0], points[i][1], points[i][2]));
}

BasicPoint toBasicPoint(const glm::vec3& point) {
    return BasicPoint(point[0], point[1], point[2]);
}

void toRawTriangleFormat(const std::vector<Triangle>& triangles, std::vector<std::vector<int>>& res) {
    res.clear();
    for(int i = 0; i < triangles.size(); ++i) {
        const Triangle& triangle = triangles[i];
        const int v1 = triangle[0];
        const int v2 = triangle[1];
        const int v3 = triangle[2];
        res.push_back(std::vector<int>{v1, v2, v3});
    }
}

void Cage::applyCage(const std::vector<glm::vec3>& cage) {
    for(int i = 0; i < cage.size(); ++i) {
        if(i < this->vertices.size())
            this->vertices[i] = cage[i];
    }
    // Artificially move a point to update the vertices position of the mesh to deform
    this->movePoint(0, this->vertices[0]);
}

void CageMVC::reInitialize() {
    this->originalVertices = this->meshToDeform->getVertices();
    this->computeNormals();
    this->MVCCoordinates.clear();
    this->computeCoordinates();
}

void CageMVC::movePoint(const int& origin, const glm::vec3& target) {
    BaseMesh::movePoint(origin, target);// Update the cage positions
    this->updateMeshToDeform();
}

void CageMVC::movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) {
    BaseMesh::movePoints(origins, targets);
    this->updateMeshToDeform();
}

void CageMVC::updateMeshToDeform() {
    // Update positions of the mesh to deform
    if(this->moveMeshToDeform) {
        std::vector<glm::vec3> newPositions(this->meshToDeform->getNbVertices(), glm::vec3(0., 0., 0.));
        for( unsigned int v = 0 ; v < this->meshToDeform->getNbVertices() ; ++v )
            for( unsigned int vc = 0 ; vc < this->MVCCoordinates[v].size() ; ++vc )
                newPositions[v] += this->MVCCoordinates[v][vc].second * this->getVertice(this->MVCCoordinates[v][vc].first);
        this->meshToDeform->replacePoints(newPositions);
    }
}

void CageMVC::computeCoordinates() {
    MVCCoordinates.clear();
    MVCCoordinates.resize(this->meshToDeform->getNbVertices());

    std::vector<std::vector<int>> trianglesRawFormat;
    std::vector<BasicPoint> verticesRawFormat;
    std::vector<BasicPoint> normalsRawFormat;

    toRawTriangleFormat(this->triangles, trianglesRawFormat);
    toBasicPoint(this->vertices, verticesRawFormat);
    toBasicPoint(this->normals, normalsRawFormat);

    for( unsigned int p_idx = 0 ; p_idx < this->meshToDeform->getNbVertices(); ++p_idx )
    {
        MVCCoords::computeMVCCoordinatesOf3dPoint(
                toBasicPoint(this->originalVertices[p_idx]),
                trianglesRawFormat,
                verticesRawFormat,
                normalsRawFormat,
                this->MVCCoordinates[p_idx]);
    }
}

void CageGreen::reInitialize() {
    this->originalVertices.clear();
    this->originalVertices = this->meshToDeform->getVertices();
    this->computeNormals();

    this->initial_cage_vertices.clear();
    this->initial_cage_triangle_normals.clear();
    this->initial_cage_vertices = this->vertices;
    this->initial_cage_triangle_normals = this->normals;

    this->phiCoordinates.clear();
    this->psiCoordinates.clear();
    this->scalingFactors.clear();

    for(int i = 0; i < this->triangles.size(); ++i) {
        const Triangle& triangle = this->triangles[i];
        const int v1 = triangle[0];
        const int v2 = triangle[1];
        const int v3 = triangle[2];
        scalingFactors.push_back(GreenCoords::GreenScalingFactor<BasicPoint>(toBasicPoint(this->vertices[v2] - this->vertices[v1]) ,toBasicPoint(this->vertices[v3] - this->vertices[v1])));
    }
    this->computeCoordinates();
}

void CageGreen::movePoint(const int& origin, const glm::vec3& target) {
    BaseMesh::movePoint(origin, target);// Update the cage positions
    this->updateMeshToDeform();
}

void CageGreen::movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) {
    BaseMesh::movePoints(origins, targets);
    this->updateMeshToDeform();
}

void CageGreen::updateMeshToDeform() {
    if(this->moveMeshToDeform) {
        // Update positions of the mesh to deform
        this->computeNormals();
        this->update_cage_triangle_scalingFactors();
        std::vector<glm::vec3> newPositions(this->meshToDeform->getNbVertices(), glm::vec3(0., 0., 0.));
        for(unsigned int v = 0; v < this->meshToDeform->getNbVertices(); ++v) {
            for(unsigned int vc = 0; vc < this->getNbVertices(); ++vc)
                newPositions[v] += static_cast<float>(phiCoordinates[v][vc]) * this->vertices[vc];
            for(unsigned int tc = 0; tc < this->triangles.size(); ++tc)
                newPositions[v] += static_cast<float>(psiCoordinates[v][tc]) * static_cast<float>(scalingFactors[tc].scalingFactor()) * glm::normalize(this->normals[tc]);

        }
        this->meshToDeform->replacePoints(newPositions);
    }
}


void CageGreen::update_cage_triangle_scalingFactors() {
    for(unsigned int t = 0; t < this->triangles.size(); ++t) {
        this->scalingFactors[t].computeScalingFactor(toBasicPoint(this->vertices[this->triangles[t].getVertex(1)] - this->vertices[this->triangles[t].getVertex(0)]),
                toBasicPoint(this->vertices[this->triangles[t].getVertex(2)] - this->vertices[this->triangles[t].getVertex(0)]));
    }
}

void CageGreen::computeCoordinates() {
    if(this->meshToDeform->getNbVertices() == 0)
        return ;
    this->phiCoordinates.clear();
    this->psiCoordinates.clear();
    this->phiCoordinates.resize(this->meshToDeform->getNbVertices());
    this->psiCoordinates.resize(this->meshToDeform->getNbVertices());

    for (unsigned int i =0; i < this->meshToDeform->getNbVertices(); i++) {
        this->phiCoordinates[i].clear();
        this->psiCoordinates[i].clear();
        this->phiCoordinates[i].resize(this->getNbVertices(), 0.f);
        this->psiCoordinates[i].resize(this->getNbVertices(), 0.f);
    }

    std::vector<std::vector<int>> trianglesRawFormat;
    std::vector<BasicPoint> verticesRawFormat;
    std::vector<BasicPoint> normalsRawFormat;

    toRawTriangleFormat(this->triangles, trianglesRawFormat);
    toBasicPoint(this->vertices, verticesRawFormat);
    toBasicPoint(this->normals, normalsRawFormat);

    for( unsigned int p_idx = 0 ; p_idx < this->meshToDeform->getNbVertices(); ++p_idx )
    {
        URAGO::computeCoordinatesOf3dPoint(
                toBasicPoint(this->originalVertices[p_idx]),
                trianglesRawFormat,
                verticesRawFormat,
                normalsRawFormat,
                this->phiCoordinates[p_idx],
                this->psiCoordinates[p_idx]);
    }
}

void CageGreenLRI::update_constraints() {
    // set values in B BasisSolver matrix:
    for( unsigned int t = 0 ; t < this->handle_tetrahedra.size() ; t ++ ){
        const std::vector<glm::vec3> & basis_def = tetInfos[handle_tetrahedra[t]].basis_def;
        for( int i = 0 ; i < 3 ; i++  )
        {
            for( int j = 0 ; j < 3 ; j++  )
            {
                BasisSolver.setValueInB( unknown_tetrahedra.size() + t, 3*i + j, basis_def[i][j]);
            }
        }
    }

    // solve BasisSolver:
    BasisSolver.solve();
    // get values:
    for( unsigned int t = 0 ; t < this->unknown_tetrahedra.size() ; t ++ ){
        std::vector<glm::vec3> basis_def(3);
        unsigned int tetra_index_in_solver = this->unknown_tetrahedra[t];
        for( int i = 0 ; i < 3 ; i++  )
        {
            for( int j = 0 ; j < 3 ; j++  )
            {
                basis_def[i][j] = BasisSolver.getSolutionValue( tetra_index_in_solver, 3*i + j);
            }
        }
        tetInfos[unknown_tetrahedra[t]].basis_def = basis_def;
    }

    // set values in B VerticesSolver matrix:
    for( unsigned int e = 0 ; e < edges.size() ; ++e )
    {
        int nTetraOnEdge = 0;
        glm::vec3 eTransform(0,0,0);
        std::vector<int> allTetIdx = this->edgeMap.at(edges[e]);
        for(int tetIdx = 0; tetIdx < allTetIdx.size(); ++tetIdx) {
            const std::vector<glm::vec3> & def_basis = this->tetInfos[tetIdx].basis_def;
            // find new orientation for edge e: eTransform = def_basis * (v1 - v0)
            glm::vec3 v0v1 = verts_initial_positions[verts_mapping_from_mesh_to_solver[edges[e].second]] - verts_initial_positions[verts_mapping_from_mesh_to_solver[edges[e].first]];
            for( int i = 0 ; i < 3 ; i ++)
                for( int j = 0 ; j < 3 ; j ++)
                    eTransform[i] += def_basis[j][i] * v0v1[j];
        }
        eTransform /= allTetIdx.size();

        for( unsigned int coord = 0 ; coord < 3 ; ++coord )
            VerticesSolver.setValueInB( e , coord , eTransform[coord] );
    }

    for( unsigned int i = 0 ; i < verts_constraints.size() ; i ++ ){
        unsigned int vertexInSolver = verts_constraints[i];
        // TODO: warning here
        glm::vec3 pos = this->getMeshToDeform()->getVertice(verts_mapping_from_solver_to_mesh[vertexInSolver]);
        for( unsigned int coord = 0 ; coord < 3 ; ++coord )
            VerticesSolver.setValueInB( edges.size() + i , coord , pos[coord] );
    }

    std::vector<int> verticesIdxToReplace;
    std::vector<glm::vec3> newVertices;

    // solve:
    VerticesSolver.solve();
    // get values:verts_mapping_from_solver_to_mesh
    double p [3];
    for( unsigned int v = 0 ; v < verts_mapping_from_solver_to_mesh.size() ; ++v ){
        for( unsigned int coord = 0 ; coord < 3 ; ++coord )
            p[coord] = VerticesSolver.getSolutionValue( v,coord );
        int vh = verts_mapping_from_solver_to_mesh[v];
        if( this->outlier_vertices[vh] ) {
            verticesIdxToReplace.push_back(vh);
            newVertices.push_back(glm::vec3(p[0], p[1], p[2]));
        }
    }
    this->meshToDeform->replacePoints(verticesIdxToReplace, newVertices);
    // free:
    BasisSolver.freeSolution();
    VerticesSolver.freeSolution();
}
