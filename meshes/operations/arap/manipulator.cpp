#include "./Manipulator.h"

namespace BasicGL {
	void glNormal3fv(glm::vec3 n) { glNormal3f(n.x, n.y, n.z); }
	void glVertex3fv(glm::vec3 n) { glVertex3f(n.x, n.y, n.z); }
	void drawSphere(float x, float y, float z, float radius, int slices, int stacks) {
		if (stacks < 2) {
			stacks = 2;
		}
		if (stacks > 20) {
			stacks = 20;
		}
		if (slices < 3) {
			slices = 3;
		}
		if (slices > 30) {
			slices = 30;
		}
		//Pas essentiel ...

		int Nb = slices * stacks + 2;
		std::vector<glm::vec3> points(Nb);

		glm::vec3 centre(x, y, z);

		float sinP, cosP, sinT, cosT, Phi, Theta;
		points[0]	   = glm::vec3(0, 0, 1);
		points[Nb - 1] = glm::vec3(0, 0, -1);

		for (int i = 1; i <= stacks; i++)
		{
			Phi	 = 90 - (float) (i * 180) / (float) (stacks + 1);
			sinP = sinf(Phi * 3.14159265f / 180.f);
			cosP = cosf(Phi * 3.14159265f / 180.f);

			for (int j = 1; j <= slices; j++)
			{
				Theta = (float) (j * 360) / (float) (slices);
				sinT  = sinf(Theta * 3.14159265f / 180.f);
				cosT  = cosf(Theta * 3.14159265f / 180.f);

				points[j + (i - 1) * slices] = glm::vec3(cosT * cosP, sinT * cosP, sinP);
			}
		}

		int k1, k2;
		glBegin(GL_TRIANGLES);
		for (int i = 1; i <= slices; i++)
		{
			k1 = i;
			k2 = (i % slices + 1);
			glNormal3fv(points[0]);
			glVertex3fv((centre + radius * points[0]));
			glNormal3fv(points[k1]);
			glVertex3fv((centre + radius * points[k1]));
			glNormal3fv(points[k2]);
			glVertex3fv((centre + radius * points[k2]));

			k1 = (stacks - 1) * slices + i;
			k2 = (stacks - 1) * slices + (i % slices + 1);
			glNormal3fv(points[k1]);
			glVertex3fv((centre + radius * points[k1]));
			glNormal3fv(points[Nb - 1]);
			glVertex3fv((centre + radius * points[Nb - 1]));
			glNormal3fv(points[k2]);
			glVertex3fv((centre + radius * points[k2]));
		}
		glEnd();

		glBegin(GL_QUADS);
		for (int j = 1; j < stacks; j++)
		{
			for (int i = 1; i <= slices; i++)
			{
				k1 = i + (j - 1) * slices;
				k2 = (i % slices + 1) + (j - 1) * slices;
				glNormal3fv(points[k2]);
				glVertex3fv((centre + radius * points[k2]));
				glNormal3fv(points[k1]);
				glVertex3fv((centre + radius * points[k1]));

				k1 = i + (j) *slices;
				k2 = (i % slices + 1) + (j) *slices;
				glNormal3fv(points[k1]);
				glVertex3fv((centre + radius * points[k1]));
				glNormal3fv(points[k2]);
				glVertex3fv((centre + radius * points[k2]));
			}
		}
		glEnd();
	}
}	 // namespace BasicGL
