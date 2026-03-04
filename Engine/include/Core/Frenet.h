#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace KGR
{
    struct CurveFrame
    {
        glm::vec3 forward;
        glm::vec3 up;
        glm::vec3 right;
    };

    namespace RMF
    {
        CurveFrame MovingFrame(const CurveFrame& previousFrame,
            const glm::vec3& from,
            const glm::vec3& to,
            const glm::vec3& nextForward);

        std::vector<CurveFrame> BuildFrames(const std::vector<glm::vec3>& points,
            const std::vector<glm::vec3>& forwardDirs);

        std::vector<glm::vec3> EstimateForwardDirs(const std::vector<glm::vec3>& points);
    }
}