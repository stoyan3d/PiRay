#pragma once

#include <glm/glm.hpp>

#include <vector>
#include "RenderObjects/RenderObject.h"

struct Material
{
	glm::vec3 Albedo{ 1.0f };
	float Roughness = 1.0f;
	float Metallic = 0.0f;
	glm::vec3 EmissionColor{ 0.0f };
	float EmissionPower = 0.0f;

	glm::vec3 GetEmission() const { return EmissionColor * EmissionPower; }
};

struct Scene
{
	std::vector<std::shared_ptr<RenderObject>> RenderObjects;
	std::vector<Material> Materials;
};