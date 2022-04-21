#include "AABBParticleContactGenerator.h"

AABBParticleContactGenerator::AABBParticleContactGenerator(const glm::vec3& minPoint, const glm::vec3& maxPoint, const glm::vec3& normal)
	: minPoint(minPoint), maxPoint(maxPoint), normal(normal)
{

}

size_t AABBParticleContactGenerator::AddContact(std::vector<Particle*>& particles, ParticleContact* contactInstance, size_t contactLimit) const
{
	size_t contactsGenerated = 0;
	for (Particle* particle : particles)
	{
		float radius = particle->GetRadius();
		glm::vec3 particlePos = particle->GetPosition();

		// Find the closest AABB point to our particle
		float closestX = std::max(this->minPoint.x, std::min(particlePos.x, this->maxPoint.x));
		float closestY = std::max(this->minPoint.y, std::min(particlePos.y, this->maxPoint.y));
		float closestZ = std::max(this->minPoint.z, std::min(particlePos.z, this->maxPoint.z));
		glm::vec3 closestPoint(closestX, closestY, closestZ);
		glm::vec3 difference = particlePos - closestPoint;

		// Since square roots are expensive, just compare everything in its squared version
		float distanceSquared = (difference.x * difference.x) + (difference.y * difference.y) + (difference.z * difference.z);
		float radiusSquared = radius * radius;

		if (distanceSquared < radiusSquared) // We have collided!
		{
			contactInstance->contactNormal = this->normal;
			contactInstance->p1 = particle;
			contactInstance->p2 = NULL;
			contactInstance->penetration = radius - sqrt(distanceSquared);
			contactInstance->restitution = 0.7f;
			contactInstance++; 
			contactsGenerated++;
			if (contactsGenerated > contactLimit)
			{
				return contactsGenerated;
			}
		}
	}

	return contactsGenerated;
}