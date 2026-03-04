#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include <stdexcept>
#include <array>
#include <glm/common.hpp>


struct HermitCurveCompute
{
public:
  static glm::vec3 ComputeSegment(float t, const glm::vec3& pStart, const glm::vec3& pEnd, const glm::vec3& pPrev, const glm::vec3& pNext, float c = 0);
private:
    static glm::vec3 ComputeM(const glm::vec3& Pminus1, const glm::vec3& Pplus1, float c);
    static  glm::vec3 ComputePoint(float t, const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& m0, const glm::vec3& m1);
    static float H00(float t);
    static float H01(float t);
    static float H10(float t);
    static float H11(float t);
};

struct HermitCurveStep
{
public:
	using vec = glm::vec3;
	using const_vec_ref = const vec&;
	HermitCurveStep() = default;
	HermitCurveStep(const_vec_ref previous, const_vec_ref start, const_vec_ref end, const_vec_ref next,float tension = 0)
	:m_points({previous, start, end, next})
	,m_tension(tension){}
	vec Compute(float t) const;
private:
	static bool IsTimeValid(float t);
	std::array<glm::vec3, 4> m_points;
	float m_tension;
};


struct HermitCurve
{
	using vec = glm::vec3;
	static HermitCurve FromPoints(const std::vector <vec>& points, float tension);
	
	HermitCurve(const std::vector<HermitCurveStep>& step);
	size_t GetStepCount() const;
	vec Compute(float t) const;

	float MaxT() const;

private:
	static bool IsTimeValid(float t,size_t vecSize);
	std::vector<HermitCurveStep> m_steps;
};
