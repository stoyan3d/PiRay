#pragma once

#include <memory>

#include "Walnut/Image.h"
#include "glm/glm.hpp"


class Renderer
{ 
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const glm::vec3 &sphereColor);

	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; };
	glm::vec4 PerPixel(glm::vec2 coord, const glm::vec3 &sphereColor);
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData;

};