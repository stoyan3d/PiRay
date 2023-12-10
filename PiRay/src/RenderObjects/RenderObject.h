#pragma once
#include "../Ray.h"

class RenderObject
{
public:
	virtual ~RenderObject() = default;

	virtual bool TraceRay(const Ray& ray, HitPayload& payload) const = 0;

public:
	glm::vec3 Position{ 0.0f };
	int MaterialIndex = 0;
};