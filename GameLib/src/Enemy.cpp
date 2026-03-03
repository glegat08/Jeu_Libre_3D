#include "Enemy.h"
#include "Tools/Random.h"

void KGR::GameLib::AIComponent::UpdateTarget(const glm::vec3& target)
{
	m_target = target;
}

glm::vec3 KGR::GameLib::AIComponent::Update(float dt, const  glm::vec3& pos, float speed)
{
	glm::vec3 result = pos;
	glm::vec3 dir = glm::normalize(m_target - pos);
	result += dir * dt;
	return result;
}

glm::vec3 KGR::GameLib::AIComponent::RandomRange(const glm::vec3& center, float minR, float maxR)
{
	float angle = KGR::Tools::Random().getRandomNumber(0.0f, 2.0f * glm::pi<float>());
	float radius = KGR::Tools::Random().getRandomNumber(minR, maxR);

	return center + glm::vec3(std::cos(angle) * radius, 0.0f, std::sin(angle) * radius);
}