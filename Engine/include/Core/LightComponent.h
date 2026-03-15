#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "Tools/Util.h"

/**
 * @brief GPU‑ready light data structure aligned for Vulkan uniform buffers.
 *
 * Contains:
 * - position / direction
 * - diffuse & specular color
 * - intensity, angle, smoothness
 * - light type (point, directional, spot)
 *
 * All vec3 fields are padded to 16 bytes to respect std140 alignment rules.
 */
struct alignas(16) LightData
{
    /**
     * @brief Supported light types.
     */
    enum class Type : std::uint32_t
    {
        Point = 0,
        Directional = 1,
        Spot = 2
    };

    PadData<glm::vec3, 16> pos = glm::vec3();          ///< Light world position.
    PadData<glm::vec3, 16> dir = glm::vec3();          ///< Light direction (normalized).
    PadData<glm::vec3, 16> lightColor = glm::vec3();   ///< Diffuse color.
    PadData<glm::vec3, 16> specularColor = glm::vec3();///< Specular color.

    float intensity = 0.0f;        ///< Light intensity (point/spot only).
    float openAngle = 0.0f;        ///< Spot light cone angle (radians).
    float additionalSmooth = 0.0f; ///< Spot light smooth falloff.
    float shiny = 0.0f;            ///< Specular shininess factor.

    PadData<uint32_t, 16> type =
        static_cast<std::uint32_t>(LightData::Type::Point); ///< Light type.
};


/**
 * @brief Base template for light components.
 *
 * Specialized for:
 * - Point
 * - Directional
 * - Spot
 */
template<LightData::Type type>
struct LightComponent;


// ============================================================================
// POINT LIGHT
// ============================================================================

/**
 * @brief Component representing a point light.
 *
 * Stores:
 * - diffuse & specular color
 * - intensity
 * - shininess
 */
template<>
struct LightComponent<LightData::Type::Point>
{
    glm::vec3 GetLightColor() const;
    glm::vec3 GetSpecularColor() const;
    float GetIntensity() const;
    float GetShiny() const;

    /**
     * @brief Sets diffuse and specular colors.
     */
    void SetColor(const glm::vec3& lightColor, const glm::vec3& specularColor);

    /**
     * @brief Sets intensity and shininess.
     */
    void SetParam(float intensity, float shiny);

    /**
     * @brief Converts this component to GPU‑ready LightData.
     */
    LightData ToData() const;

    /**
     * @brief Factory method for convenience.
     */
    static LightComponent Create(const glm::vec3& lightColor,
        const glm::vec3& specularColor,
        float intensity,
        float shiny);

private:
    glm::vec3 m_lightColor = glm::vec3{ 1.0f };
    glm::vec3 m_specularColor = glm::vec3{ 1.0f };
    float m_intensity = 100.0f;
    float m_shiny = 100.0f;
};


// ============================================================================
// DIRECTIONAL LIGHT
// ============================================================================

/**
 * @brief Component representing a directional light.
 *
 * Stores:
 * - diffuse & specular color
 * - shininess
 *
 * Direction is provided by the entity's TransformComponent.
 */
template<>
struct LightComponent<LightData::Type::Directional>
{
    glm::vec3 GetLightColor() const;
    glm::vec3 GetSpecularColor() const;
    float GetShiny() const;

    /**
     * @brief Sets diffuse and specular colors.
     */
    void SetColor(const glm::vec3& lightColor, const glm::vec3& specularColor);

    /**
     * @brief Sets shininess.
     */
    void SetParam(float shiny);

    /**
     * @brief Converts this component to GPU‑ready LightData.
     */
    LightData ToData() const;

    /**
     * @brief Factory method for convenience.
     */
    static LightComponent Create(const glm::vec3& lightColor,
        const glm::vec3& specularColor,
        float shiny);

private:
    glm::vec3 m_lightColor = glm::vec3{ 1.0f };
    glm::vec3 m_specularColor = glm::vec3{ 1.0f };
    float m_shiny = 100.0f;
};


// ============================================================================
// SPOT LIGHT
// ============================================================================

/**
 * @brief Component representing a spot light.
 *
 * Stores:
 * - diffuse & specular color
 * - intensity
 * - shininess
 * - cone angle
 * - smooth falloff
 */
template<>
struct LightComponent<LightData::Type::Spot>
{
    glm::vec3 GetLightColor() const;
    glm::vec3 GetSpecularColor() const;
    float GetIntensity() const;
    float GetShiny() const;
    float GetOpenAngle() const;
    float GetAdditionalSmooth() const;

    /**
     * @brief Sets diffuse and specular colors.
     */
    void SetColor(const glm::vec3& lightColor, const glm::vec3& specularColor);

    /**
     * @brief Sets intensity and shininess.
     */
    void SetParam(float intensity, float shiny);

    /**
     * @brief Sets the spot cone angle and smooth falloff.
     *
     * @param angle Cone angle in radians.
     * @param additionalSmooth Extra smoothing factor.
     */
    void SetAngle(float angle, float additionalSmooth);

    /**
     * @brief Converts this component to GPU‑ready LightData.
     */
    LightData ToData() const;

    /**
     * @brief Factory method for convenience.
     */
    static LightComponent Create(const glm::vec3& lightColor,
        const glm::vec3& specularColor,
        float intensity,
        float shiny,
        float openAngle,
        float addSmooth);

private:
    glm::vec3 m_lightColor = glm::vec3{ 1.0f };
    glm::vec3 m_specularColor = glm::vec3{ 1.0f };
    float m_intensity = 100.0f;
    float m_shiny = 100.0f;
    float m_openAngle = glm::radians(45.0f);
    float m_additionalSmooth = 5.0f;
};