#include "Renderer.h"
#include "Walnut/Random.h"
#include <execution>

namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = static_cast<uint8_t>(color.r * 255.0f);
		uint8_t g = static_cast<uint8_t>(color.g * 255.0f);
		uint8_t b = static_cast<uint8_t>(color.b * 255.0f);
		uint8_t a = static_cast<uint8_t>(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}

	static uint32_t PCG_Hash(uint32_t input)
	{
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	static float RandomFloat(uint32_t& seed)
	{
		seed = PCG_Hash(seed);
		return (float)seed / (float)std::numeric_limits<uint32_t>::max();
	}

	static glm::vec3 InUnitSphere(uint32_t& seed)
	{
		return glm::normalize(glm::vec3(
			RandomFloat(seed) * 2.0f - 1.0f,
			RandomFloat(seed) * 2.0f - 1.0f,
			RandomFloat(seed) * 2.0f - 1.0f
		));
	}

	static glm::vec4 LinearToGamma(const glm::vec4& color)
	{
		// TODO Use a pow instead if we want a more accurate conversion but it takes longer to compute
		return glm::vec4{
			glm::sqrt(color.r),
			glm::sqrt(color.g),
			glm::sqrt(color.b),
			color.a
		};
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);
	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIter[i] = i;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

#define MT 1
#if MT
	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this](uint32_t y) 
		{
			std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
				[this, y](uint32_t x)
				{
					glm::vec4 color = PerPixel(x, y);
					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

					glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
					accumulatedColor /= static_cast<float>(m_FrameIndex);

					accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
					accumulatedColor = Utils::LinearToGamma(accumulatedColor);
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
				});
		});
#else
	for (uint32_t y{ 0 }; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x{ 0 }; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 color = PerPixel(x, y);
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			accumulatedColor /= static_cast<float>(m_FrameIndex);

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
		}
	}
#endif

	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
		m_FrameIndex++;
	else
		ResetFrameIndex();
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray{};
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];
	
	glm::vec3 light{ 0.0f };
	glm::vec3 contribution{ 1.0f };
	uint32_t seed = x + y * m_FinalImage->GetWidth();
	seed *= m_FrameIndex;

	int bounces = 3;
	for (int i = 0; i < bounces; i++)
	{
		seed += m_FrameIndex;

		HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0)
		{
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			// light += skyColor * contribution;
			break;
		}

		//glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
		//float directLight = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f);

		const auto& sphere = m_ActiveScene->RenderObjects[payload.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere->MaterialIndex];

		glm::vec3 sphereColor = material.Albedo;
		//sphereColor *= directLight;
		//light += sphereColor * contribution;

		contribution *= material.Albedo;
		light += material.GetEmission();

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		//ray.Direction = glm::reflect(ray.Direction, 
		//	payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
		ray.Direction = glm::normalize(Utils::InUnitSphere(seed) + payload.WorldNormal);
	}

	return glm::vec4{ light, 1.0f };
}

HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const auto& closestRenderObject = m_ActiveScene->RenderObjects[objectIndex];

	glm::vec3 origin = ray.Origin - closestRenderObject->Position;
	payload.WorldPosition = origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	payload.WorldPosition += closestRenderObject->Position;

	return payload;
}

HitPayload Renderer::Miss(const Ray& ray)
{
	HitPayload payload{};
	payload.HitDistance = -1.0f;
	return payload;
}

HitPayload Renderer::TraceRay(const Ray& ray)
{
	HitPayload payload;
	// (bx^2 + by^2)t^2 + 2(axbx + ayby)t + ax^2 + ay^2 - r^2 = 0;

	int closestRenderObject = -1;
	float hitDistance = std::numeric_limits<float>::max();
	for (size_t i = 0; i < m_ActiveScene->RenderObjects.size(); i++)
	{
		auto& renderObject = m_ActiveScene->RenderObjects[i];

		if (!renderObject->TraceRay(ray, payload))
			continue;

		if (payload.HitDistance > 0.0f && payload.HitDistance < hitDistance)
		{
			hitDistance = payload.HitDistance;
			closestRenderObject = static_cast<int>(i);
		}
	}

	if (closestRenderObject < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestRenderObject);
}
