#include "Sphere.h"

bool Sphere::TraceRay(const Ray& ray, HitPayload& payload) const
{
	glm::vec3 origin = ray.Origin - Position;

	float a = glm::dot(ray.Direction, ray.Direction);
	float b = 2.0f * glm::dot(origin, ray.Direction);
	float c = glm::dot(origin, origin) - Radius * Radius;

	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f)
		return false;

	float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
	payload.HitDistance = closestT;
	
	return true;
}
