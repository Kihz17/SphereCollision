#include "MeshParticleContactGenerator.h"
#include "Mesh.h"

#include <glm/gtx/norm.hpp> // length2
#include <iostream>

MeshParticleContactGenerator::MeshParticleContactGenerator(Model* model, const glm::vec3& translation)
	: model(model), translation(translation)
{

}

size_t MeshParticleContactGenerator::AddContact(std::vector<Particle*>& particles, ParticleContact* contactInstance, size_t contactLimit) const
{
	size_t contactsGenerated = 0;
	for (Particle* particle : particles)
	{
		glm::vec3 particlePos = particle->GetPosition();
		float radius = particle->GetRadius();
		float radius2 = radius * radius;

		for (const Mesh& mesh : this->model->GetMeshes())
		{
			// Get the geometry of the current mesh
			const std::vector<sVertex>& vertices = mesh.GetVertices();
			const std::vector<sTriangle>& faces = mesh.GetFaces();
			int numFaces = faces.size();

			for (int i = 0; i < numFaces; i++)
			{
				sTriangle face = faces[i];
				sVertex vertex1 = vertices[face.vertIndex[0]];
				sVertex vertex2 = vertices[face.vertIndex[1]];
				sVertex vertex3 = vertices[face.vertIndex[2]];

				glm::vec3 vertexPos1 = glm::vec3(vertex1.x, vertex1.y, vertex1.z) + translation;
				glm::vec3 vertexPos2 = glm::vec3(vertex2.x, vertex2.y, vertex2.z) + translation;
				glm::vec3 vertexPos3 = glm::vec3(vertex3.x, vertex3.y, vertex3.z) + translation;

				glm::vec3 normal1(vertex1.nx, vertex1.ny, vertex1.nz);
				glm::vec3 normal2(vertex2.nx, vertex2.ny, vertex2.nz);
				glm::vec3 normal3(vertex3.nx, vertex3.ny, vertex3.nz);
				glm::vec3 normal = (normal1 + normal2 + normal3) / 3.0f; // Average 3 normals to get face normal

				glm::vec3 vertexPositionSum = vertexPos1 + vertexPos2 + vertexPos3;
				float distance = -glm::dot(vertexPositionSum / 3.0f, normal);
				float pointToPlaneDistance = glm::dot(normal, particlePos) + distance;

				if (fabs(pointToPlaneDistance) > radius) // We are not breaking this polygon's infinite plane, no point in checking any further
				{
					continue;
				}

				// Check if our particle is within the face's triangle (https://blackpawn.com/texts/pointinpoly/default.html)
				auto IsSameSide = [](glm::vec3 p1, glm::vec3 p2, glm::vec3 a, glm::vec3 b)
				{
					return glm::dot(glm::cross(b - a, p1 - a), glm::cross(b - a, p2 - a)) >= 0.0f;
				};

				bool inside = IsSameSide(particlePos, vertexPos1, vertexPos2, vertexPos3) && IsSameSide(particlePos, vertexPos2, vertexPos1, vertexPos3) && IsSameSide(particlePos, vertexPos3, vertexPos1, vertexPos2);
				if (!inside)
				{
					continue;
				}

				// We have made contact!
				contactInstance->contactNormal = normal;
				contactInstance->p1 = particle;
				contactInstance->p2 = NULL;
				contactInstance->penetration = radius - pointToPlaneDistance;
				contactInstance->restitution = 0.7f;
				contactInstance++;
				contactsGenerated++;
				if (contactsGenerated > contactLimit)
				{
					return contactsGenerated;
				}

				break; // We have already collided, no point in continuing to look
			}
		}

	}

	return contactsGenerated;
}
