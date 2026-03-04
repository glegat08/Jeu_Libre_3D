#include "Core/Frenet.h"
#include <stdexcept>

KGR::CurveFrame KGR::RMF::MovingFrame(const CurveFrame& previousFrame, const glm::vec3& from, const glm::vec3& to,
										 const glm::vec3& nextForward)
{
    glm::vec3 segmentDirection = glm::normalize(to - from);

	// Reflection formula: v' = v - 2 * dot(v, n) * n
    glm::vec3 reflectionForward = previousFrame.forward - 2.0f * glm::dot(previousFrame.forward, segmentDirection) * segmentDirection;
    glm::vec3 reflectionUp = previousFrame.up - 2.0f * glm::dot(previousFrame.up, segmentDirection) * segmentDirection;

    glm::vec3 correctionAxis = nextForward - reflectionForward;
    float     correctionSquare = glm::dot(correctionAxis, correctionAxis);

    glm::vec3 movingUp = (correctionSquare < 1e-10f) ? reflectionUp : reflectionUp - (2.0f / correctionSquare) * glm::dot(correctionAxis, reflectionUp) * correctionAxis;
    
    KGR::CurveFrame nextFrame;
    nextFrame.forward = nextForward;
    nextFrame.up = glm::normalize(movingUp);
    nextFrame.right = glm::normalize(glm::cross(nextFrame.forward, nextFrame.up));

    return nextFrame;
}

glm::vec3 KGR::RMF::EstimateForwardDir(const std::optional<glm::vec3>& prev, const glm::vec3& current, const std::optional<glm::vec3>& next)
{
    const glm::vec3 fromPrev = prev ? glm::normalize(current - *prev) : glm::vec3(0);
    const glm::vec3 toNext = next ? glm::normalize(*next - current) : glm::vec3(0);

    if (!prev) 
        return toNext;

    if (!next) 
        return fromPrev;

    return glm::normalize(fromPrev + toNext);
}

std::vector<glm::vec3> KGR::RMF::EstimateForwardDirs(const std::vector<glm::vec3>& points)
{
    const std::size_t pointCount = points.size();

    if (pointCount < 2)
        throw std::invalid_argument("at least 2 points required");

    std::vector<glm::vec3> tangents(pointCount);
    for (std::size_t i = 0; i < pointCount; ++i)
    {
        auto prev = (i > 0) ? std::optional(points[i - 1]) : std::nullopt;
        auto next = (i < pointCount - 1) ? std::optional(points[i + 1]) : std::nullopt;
        tangents[i] = EstimateForwardDir(prev, points[i], next);
    }

    return tangents;
}

std::vector<KGR::CurveFrame> KGR::RMF::BuildFrames(const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& tangents)
{
    const std::size_t pointCount = points.size();

    if (pointCount < 2 || tangents.size() != pointCount)
        throw std::invalid_argument("points and tangents must have the same size (>= 2)");

    std::vector<CurveFrame> frames(pointCount);

    glm::vec3 worldAxis = (std::abs(tangents[0].y) < 0.9f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);

    frames[0].forward = tangents[0];
    frames[0].up = glm::normalize(worldAxis - glm::dot(worldAxis, tangents[0]) * tangents[0]);
    frames[0].right = glm::normalize(glm::cross(frames[0].forward, frames[0].up));

    for (std::size_t i = 0; i < pointCount - 1; ++i)
        frames[i + 1] = MovingFrame(frames[i], points[i], points[i + 1], tangents[i + 1]);

    return frames;
}

KGR::CurveFrame KGR::RMF::InterpolateFrame(const CurveFrame& a, const CurveFrame& b, float t)
{
    CurveFrame frame;

	// glm::mix performs linear interpolation between two vectors, and we normalize the results to ensure they remain unit vectors.
    frame.forward = glm::normalize(glm::mix(a.forward, b.forward, t));
    frame.up = glm::normalize(glm::mix(a.up, b.up, t));
    frame.right = glm::normalize(glm::cross(frame.forward, frame.up));

    return frame;
}