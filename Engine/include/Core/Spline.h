#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include <stdexcept>
#include <array>
#include <glm/common.hpp>

/**
 * @brief Static utility class for computing Hermite curve segments.
 *
 * Provides:
 * - tangent computation using Catmull–Rom style formula
 * - Hermite basis functions (H00, H01, H10, H11)
 * - interpolation between two points using tension
 */
struct HermitCurveCompute
{
public:
    /**
     * @brief Computes a point on a Hermite segment.
     *
     * @param t Interpolation parameter in [0, 1].
     * @param pStart Starting point of the segment.
     * @param pEnd Ending point of the segment.
     * @param pPrev Previous control point (for tangent computation).
     * @param pNext Next control point (for tangent computation).
     * @param c Tension parameter (0 = Catmull–Rom).
     * @return Interpolated point on the curve.
     */
    static glm::vec3 ComputeSegment(float t,
        const glm::vec3& pStart,
        const glm::vec3& pEnd,
        const glm::vec3& pPrev,
        const glm::vec3& pNext,
        float c = 0);

private:
    /**
     * @brief Computes the tangent vector at a point using Catmull–Rom style formula.
     *
     * @param Pminus1 Previous point.
     * @param Pplus1 Next point.
     * @param c Tension parameter.
     */
    static glm::vec3 ComputeM(const glm::vec3& Pminus1,
        const glm::vec3& Pplus1,
        float c);

    /**
     * @brief Computes the Hermite interpolation between two points.
     *
     * @param t Interpolation parameter.
     * @param P0 Start point.
     * @param P1 End point.
     * @param m0 Tangent at P0.
     * @param m1 Tangent at P1.
     */
    static glm::vec3 ComputePoint(float t,
        const glm::vec3& P0,
        const glm::vec3& P1,
        const glm::vec3& m0,
        const glm::vec3& m1);

    /// @brief Hermite basis function H00(t).
    static float H00(float t);

    /// @brief Hermite basis function H01(t).
    static float H01(float t);

    /// @brief Hermite basis function H10(t).
    static float H10(float t);

    /// @brief Hermite basis function H11(t).
    static float H11(float t);
};


/**
 * @brief Represents a single Hermite curve segment.
 *
 * Stores:
 * - previous point
 * - start point
 * - end point
 * - next point
 * - tension parameter
 *
 * Provides a Compute(t) method to evaluate the segment.
 */
struct HermitCurveStep
{
public:
    using vec = glm::vec3;
    using const_vec_ref = const vec&;

    HermitCurveStep() = default;

    /**
     * @brief Constructs a Hermite segment from four control points.
     *
     * @param previous Point before the segment start.
     * @param start Segment start point.
     * @param end Segment end point.
     * @param next Point after the segment end.
     * @param tension Tension parameter (0 = Catmull–Rom).
     */
    HermitCurveStep(const_vec_ref previous,
        const_vec_ref start,
        const_vec_ref end,
        const_vec_ref next,
        float tension = 0)
        : m_points({ previous, start, end, next })
        , m_tension(tension)
    {
    }

    /**
     * @brief Computes a point on the segment.
     *
     * @param t Interpolation parameter in [0, 1].
     * @return Interpolated point.
     */
    vec Compute(float t) const;

private:
    /// @brief Checks if t is within [0, 1].
    static bool IsTimeValid(float t);

    std::array<glm::vec3, 4> m_points; ///< {previous, start, end, next}
    float m_tension;                   ///< Tension parameter.
};


/**
 * @brief Represents a full Hermite curve composed of multiple segments.
 *
 * Provides:
 * - construction from a list of control points
 * - evaluation at global time t
 * - access to segment count and max time
 */
struct HermitCurve
{
    using vec = glm::vec3;

    /**
     * @brief Creates a Hermite curve from a list of control points.
     *
     * @param points Control points.
     * @param tension Tension parameter for all segments.
     */
    static HermitCurve FromPoints(const std::vector<vec>& points, float tension);

    /**
     * @brief Constructs a Hermite curve from precomputed steps.
     */
    HermitCurve(const std::vector<HermitCurveStep>& step);
    HermitCurve() = default;
    /// @brief Returns the number of segments.
    size_t GetStepCount() const;

    /**
     * @brief Computes a point on the full curve.
     *
     * @param t Global time in [0, MaxT()].
     */
    vec Compute(float t) const;

    /**
     * @brief Returns the maximum valid t value (equal to number of segments).
     */
    float MaxT() const;

private:
    /// @brief Validates global time t.
    static bool IsTimeValid(float t, size_t vecSize);

    std::vector<HermitCurveStep> m_steps; ///< All curve segments.
};