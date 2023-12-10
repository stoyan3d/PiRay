#pragma once

#include "RenderObject.h"

class Sphere : public RenderObject
{
public:
	float Radius = 0.5f;

	virtual bool TraceRay(const Ray& ray, HitPayload& payload) const override;
};