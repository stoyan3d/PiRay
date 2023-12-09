#pragma once

#include <memory>

#include "Walnut/Image.h"
#include "glm/glm.hpp"
#include "Camera.h"
#include "Ray.h"


class Renderer
{ 
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const glm::vec3 &sphereColor, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; };
private:
	glm::vec4 TraceRay(const Ray& ray, const glm::vec3 &sphereColor);
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

};