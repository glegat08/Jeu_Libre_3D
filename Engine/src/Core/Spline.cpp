#include "Core/Spline.h"


glm::vec3 HermitCurveCompute::ComputeSegment(float t, const glm::vec3& pPrev, const glm::vec3& pStart, const glm::vec3& pEnd,
	const glm::vec3& pNext, float c)
{
	glm::vec3 m0 = ComputeM(pPrev, pEnd, c);
	glm::vec3 m1 = ComputeM(pStart, pNext, c);

	return ComputePoint(t, pStart, pEnd, m0, m1);
}

glm::vec3 HermitCurveCompute::ComputeM(const glm::vec3& Pminus1, const glm::vec3& Pplus1, float c)
{
	return (1 - c) * ((Pplus1 - Pminus1) / 2.0f);
}

glm::vec3 HermitCurveCompute::ComputePoint(float t, const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& m0,
	const glm::vec3& m1)
{
	float h00 = H00(t);
	float h10 = H10(t);
	float h01 = H01(t);
	float h11 = H11(t);

	return (h00 * P0 + h10 * m0 + h01 * P1 + h11 * m1);
}

float HermitCurveCompute::H00(float t)
{
	float h00 = 2.0f * pow(t, 3.0f) - 3.0f * pow(t, 2.0f) + 1.0f;
	return h00;
}

float HermitCurveCompute::H01(float t)
{
	float h01 = -2.0f * pow(t, 3.0f) + 3.0f * pow(t, 2.0f);
	return h01;
}

float HermitCurveCompute::H10(float t)
{
	float h10 = pow(t, 3.0f) - 2 * pow(t, 2.0f) + t;
	return h10;
}

float HermitCurveCompute::H11(float t)
{
	float h11 = pow(t, 3.0f) - pow(t, 2.0f);
	return h11;
}

HermitCurveStep::vec HermitCurveStep::Compute(float t) const
{
	if (!IsTimeValid(t))
		throw std::out_of_range("time Invalid must be between 0 and 1");

	return HermitCurveCompute::ComputeSegment(t,m_points[0], m_points[1], m_points[2], m_points[3], m_tension);
}

bool HermitCurveStep::IsTimeValid(float t)
{
	return t >= 0 && t <= 1;
}


HermitCurve HermitCurve::FromPoints(const std::vector<vec>& points, float tension = 0.0f)
{
	if (points.size() < 4)
		throw std::out_of_range("must be at least 4 points");
	std::vector<HermitCurveStep> step;
	for (int i = 1; i < points.size() - 2; ++i)
		step.emplace_back(points[i - 1], points[i], points[i + 1], points[i + 2], tension);
	return HermitCurve{step};
}

HermitCurve::HermitCurve(const std::vector<HermitCurveStep>& step): m_steps(step)
{}

HermitCurve::vec HermitCurve::Compute(float t) const
{
	if (!IsTimeValid(t,m_steps.size()))
	{
		throw std::out_of_range("must be between 0 and the stepCount");
	}


	int floor = static_cast<int>(std::floor(t));
	floor = std::min(floor, static_cast<int>(m_steps.size()) - 1);
	float fract = t - static_cast<float>(floor);
	return m_steps[floor].Compute(fract);
}

float HermitCurve::MaxT() const
{
	return static_cast<float>(m_steps.size());
}

bool HermitCurve::IsTimeValid(float t, size_t vecSize)
{

	return vecSize > 0 && t >= 0 && t <= static_cast<float>(vecSize);
}



size_t HermitCurve::GetStepCount() const
{
	return m_steps.size();
}
