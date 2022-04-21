#pragma once

#include "IParticleContactGenerator.h"

class AABBParticleContactGenerator : public IParticleContactGenerator
{
public:
	AABBParticleContactGenerator(const glm::vec3& minPoint, const glm::vec3& maxPoint, const glm::vec3& normal);
	~AABBParticleContactGenerator() = default;

	virtual size_t AddContact(std::vector<Particle*>& particles, ParticleContact* contactInstance, size_t contactLimit) const override;

private:
	glm::vec3 minPoint;
	glm::vec3 maxPoint;
	glm::vec3 normal;
};