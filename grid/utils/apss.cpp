#include "apss.hpp"
#include<algorithm>
#include<glm/gtx/norm.hpp>

ProjectedPoint apss(const glm::vec3& inputPoint, 
                    const std::vector<glm::vec3>& positions, 
                    const std::vector<glm::vec3>& normals,
                    const std::vector<int>& knn_indices,// nth closest points from inputPoint
                    const std::vector<float>& knn_squared_distances) {

	ProjectedPoint output;
	output.position = inputPoint;
	
	static auto phif = [](auto x) {return (x < 1) ? std::pow((1.f - (x * x)), 4) : 0; }; //fct pour calculer les wi

	float s_wi = 0.f;
	float s_wipini = 0.f;
	float s_wipipi = 0.f;
	glm::vec3 s_wipi(0.f, 0.f, 0.f);
	glm::vec3 s_wini(0.f, 0.f, 0.f);

	float curvature_estimation = 0.0f;

	//const float max_dist = std::sqrt(knn_squared_distances.maxCoeff());
	const float max_dist = std::sqrt(*max_element(knn_squared_distances.begin(), knn_squared_distances.end()));
	for (unsigned int i = 0; i < knn_indices.size(); i++) {
        if (knn_indices[i] >= 0 && knn_indices[i] < positions.size()) {
			//skip invalid index
			const unsigned int nni_idx = knn_indices[i];
			const float nni_sqrDist = knn_squared_distances[i];
			const float d = std::sqrt(nni_sqrDist);
			//float wi = std::pow(1-d/max_dist, 4) * (1+4*d/max_dist); // la fonction Wendland
			const float wi = phif(d / max_dist); // la fonction papier
			//double wi = std::pow(max_dist/std::sqrt(nni_sqrDist), 2); //la fonction singulier
			const glm::vec3& pos = positions[nni_idx];
			const glm::vec3& normal = normals[nni_idx];
            s_wipini += (wi * glm::dot(pos, normal));
            s_wipipi += (wi * glm::dot(pos, pos));
			s_wipi += (wi * pos);
			s_wini += (wi * normal);
			s_wi += wi;
		}
	}
	// algebraic sphere: u4.||X||^2 + u123.X + u0 = 0
	// geometric sphere: ||X-C||^2 - r^2 = 0
	// geometric plane:  (X-C).n = 0
    // WARNING
    //const float u4 = 0.5F * (s_wipini / s_wi - (s_wipi / s_wi).dot(s_wini / s_wi)) / (s_wipipi / s_wi - (s_wipi / s_wi).dot(s_wipi / s_wi));
    const float u4 = 0.5F * (glm::dot(s_wipini / s_wi - (s_wipi / s_wi), s_wini / s_wi)) / (glm::dot(s_wipipi / s_wi - (s_wipi / s_wi), s_wipi / s_wi));
    const glm::vec3 u123 = (s_wini - 2 * u4 * s_wipi) / s_wi;
    const float u0 = -(glm::dot(s_wipi, u123) + u4 * s_wipipi) / s_wi;
	if (std::fabs(u4) < 0.000000000001F) {
		// then project on a plane (it's a degenerate sphere)
		const glm::vec3 n = -u123;
        // WARNING
        //const float lambda = (u0 - glm::dot(output.position, n)) / n.squaredNorm();
        const float lambda = (u0 - glm::dot(output.position, n)) / (glm::l2Norm(n)*glm::l2Norm(n));
        output.position += lambda * n;
		output.normal = s_wini;
        output.normal = glm::normalize(output.normal);
	}
	else {
		const glm::vec3 sphere_center = u123 / (-2.0F * u4);
        const float sphere_radius = std::sqrt(std::max<float>(0.0F, (glm::l2Norm(sphere_center) * glm::l2Norm(sphere_center)) - u0 / u4));
		// projection of the inputpoint onto the sphere
		// direction of the point from the sphere center
		glm::vec3 pc = output.position - sphere_center;
        pc = glm::normalize(pc);

		output.position = sphere_center + sphere_radius * pc;
        output.normal = u123 + 2.0f * u4 * output.position;
        output.normal = glm::normalize(output.normal);

		if (std::fabs(sphere_radius) > 1e-3f) {
			curvature_estimation = 1.f / sphere_radius;
			if (u4 < 0.f) {
				curvature_estimation *= -1.f;
			}
		}

	}
	output.curvature = curvature_estimation;
	return output;
}
