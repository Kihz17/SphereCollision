#pragma once

#include "IParticleContactGenerator.h"
#include "Model.h"

class MeshParticleContactGenerator : public IParticleContactGenerator
{
public:
	MeshParticleContactGenerator(Model* model, const glm::vec3& translation);
	~MeshParticleContactGenerator() = default;

	virtual size_t AddContact(std::vector<Particle*>& particles, ParticleContact* contactInstance, size_t contactLimit) const override;

private:
	Model* model;
	glm::vec3 translation;
};