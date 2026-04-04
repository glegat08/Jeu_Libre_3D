#include "Math/AABB2D.h"

KGR::AABB2D::AABB2D(const glm::vec2& min, const glm::vec2& max):Amin(min), Amax(max)
{}

KGR::AABB2D::AABB2D() = default;

glm::vec2 KGR::AABB2D::GetSize() const
{
	return (Amax - Amin);
}

glm::vec2 KGR::AABB2D::GetCenter() const
{
	return Amin + (GetSize() / 2.0f);
}

KGR::AABB2D KGR::AABB2D::FromPoints(const std::vector<glm::vec2>& points)
{
	AABB2D result;
	if (points.size() < 2)
		throw std::out_of_range("need at least 2 points");

	result.Amin = points.front();
	result.Amax = points.front();


	for (int i = 1; i < points.size(); ++i)
		result.Extend(points[i]);

	return result;
}

void KGR::AABB2D::Extend(const glm::vec2& point)
{
	Amin.x = std::min(Amin.x, point.x);
	Amin.y = std::min(Amin.y, point.y);
	Amax.x = std::max(Amax.x, point.x);
	Amax.y = std::max(Amax.y, point.y);
}

bool KGR::AABB2D::operator==(const AABB2D& other) const
{
	return Amax == other.Amax && Amin == other.Amin;
}

bool KGR::AABB2D::IsColliding(const glm::vec2& point) const
{
	if (point.x < Amin.x || point.x > Amax.x || point.y < Amin.y || point.y > Amax.y)
		return false;
	return true;
}

bool KGR::AABB2D::IsColliding(const AABB2D& other) const
{
	if (Amin.x > other.Amax.x || Amax.x < other.Amin.x || Amin.y > other.Amax.y || Amax.y < other.Amin.y)
		return false;
	return true;
}
