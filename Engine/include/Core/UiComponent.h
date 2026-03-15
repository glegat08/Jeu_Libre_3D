#pragma once 
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

/**
 * @brief Represents a UI element with position, scale, anchor, and color.
 *
 * This component stores information for a UI element in a virtual resolution space,
 * and provides methods to convert between virtual resolution (VR) and normalized device coordinates (NDC).
 */
struct UiComponent
{
    /**
     * @brief Defines the anchor point of the UI element.
     *
     * The anchor determines how the element is positioned relative to its reference point.
     */
    enum class Anchor
    {
        LeftTop,    /**< Anchor at the top-left corner */
        RightTop,   /**< Anchor at the top-right corner */
        Center,     /**< Anchor at the center */
        LeftBottom, /**< Anchor at the bottom-left corner */
        RightBottom /**< Anchor at the bottom-right corner */
    };

    /**
     * @brief Converts a 2D vector from virtual resolution space to normalized device coordinates (NDC).
     * @param vec Position in virtual resolution.
     * @param vr Virtual resolution of the UI context.
     * @param aspectRatio Aspect ratio of the viewport.
     * @param scale Whether to apply scale conversion.
     * @return Position in NDC.
     */
    static glm::vec2 VrToNdc(const glm::vec2& vec, const glm::vec2& vr, float aspectRatio, bool scale);

    /**
     * @brief Converts a 2D vector from NDC to virtual resolution space.
     * @param vec Position in NDC.
     * @param vr Virtual resolution of the UI context.
     * @param aspectRatio Aspect ratio of the viewport.
     * @param scale Whether to apply scale conversion.
     * @return Position in virtual resolution.
     */
    static glm::vec2 NdcToVr(const glm::vec2& vec, const glm::vec2& vr, float aspectRatio, bool scale);

    /** @brief Converts X coordinate from virtual resolution to NDC. */
    static float VrToNdcX(float x, float vrX, float aspectRatio, bool scale);

    /** @brief Converts Y coordinate from virtual resolution to NDC. */
    static float VrToNdcY(float y, float vrY, bool scale);

    /** @brief Converts X coordinate from NDC to virtual resolution. */
    static float NdcToVrX(float x, float vrX, float aspectRatio, bool scale);

    /** @brief Converts Y coordinate from NDC to virtual resolution. */
    static float NdcToVrY(float y, float vrY, bool scale);

    /** @brief Default constructor. Initializes the component with default virtual resolution, position, scale, anchor, and color. */
    UiComponent() = default;

    /**
     * @brief Constructor with virtual resolution and anchor.
     * @param vr Virtual resolution (width, height) of the UI element.
     * @param anchor Anchor point to use.
     */
    UiComponent(const glm::vec2& vr, Anchor anchor);

    /** @brief Sets the virtual resolution. */
    void SetVr(const glm::vec2& vr);

    /** @brief Gets the virtual resolution. */
    glm::vec2 GetVr() const;

    /** @brief Sets the position in virtual resolution coordinates. */
    void SetPos(const glm::vec2& pos);

    /** @brief Gets the position in virtual resolution coordinates. */
    glm::vec2 GetPosVr() const;

    /**
     * @brief Gets the position converted to normalized device coordinates (NDC).
     * @param aspectRatio Aspect ratio of the viewport.
     */
    glm::vec2 GetPosNdc(float aspectRatio) const;

    /** @brief Sets the scale in virtual resolution space. */
    void SetScale(const glm::vec2& scale);

    /** @brief Gets the scale in virtual resolution space. */
    glm::vec2 GetScaleVr() const;

    /** @brief Gets the scale converted to normalized device coordinates (NDC). */
    glm::vec2 GetScaleNdc(float aspectRatio) const;

    /** @brief Sets the anchor point of the UI element. */
    void SetAnchor(Anchor anchor);

    /** @brief Gets the anchor point of the UI element. */
    Anchor GetAnchor() const;

    /** @brief Sets the color of the UI element (RGBA). */
    void SetColor(const glm::vec4& color);

    /** @brief Gets the color of the UI element (RGBA). */
    glm::vec4 GetColor() const;

private:
    /**
     * @brief Applies an offset based on the anchor to adjust the position.
     * @param pos Original position.
     * @param scale Scale of the UI element.
     * @param anchor Anchor point.
     * @return Adjusted position.
     */
    static glm::vec2 applyOffSet(const glm::vec2& pos, const glm::vec2& scale, Anchor anchor);

    glm::vec2 m_virtualRes = { 1920,1080 }; /**< Virtual resolution (width, height) of the UI context */
    glm::vec2 m_pos = { 0,0 };              /**< Position in virtual resolution coordinates */
    glm::vec2 m_scale = { 1,1 };            /**< Scale of the UI element */
    Anchor m_anchor = Anchor::Center;     /**< Anchor point for positioning */
    glm::vec4 m_color = { 1.0f,1.0f,1.0,1.0f }; /**< Color (RGBA) of the UI element */
};