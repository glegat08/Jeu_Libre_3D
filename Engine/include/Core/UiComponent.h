#pragma once 
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct UiComponent
{
	enum class Anchor
	{
		LeftTop,
		RightTop,
		Center,
		LeftBottom,
		RightBottom
	};
	static glm::vec2 VrToNdc(const glm::vec2& vec, const glm::vec2& vr, float aspectRatio,bool scale);
	static glm::vec2 NdcToVr(const glm::vec2& vec, const glm::vec2& vr, float aspectRatio, bool scale);
	static float VrToNdcX(float x, float vrX, float aspectRatio, bool scale);
	static float VrToNdcY(float y, float vrY, bool scale);
	static float NdcToVrX(float x, float vrX, float aspectRatio, bool scale);
	static float NdcToVrY(float y, float vrY, bool scale);

	UiComponent() = default;
	UiComponent(const glm::vec2& vr,Anchor anchor);

	void SetVr(const glm::vec2& vr);
	glm::vec2 GetVr() const;

	void SetPos(const glm::vec2& pos);
	glm::vec2 GetPosVr() const;

	glm::vec2 GetPosNdc(float aspectRatio) const;
	void SetScale(const glm::vec2& scale);

	glm::vec2 GetScaleVr() const;
	glm::vec2 GetScaleNdc(float aspectRatio) const;

	void SetAnchor(Anchor anchor);
	Anchor GetAnchor() const;

	void SetColor(const glm::vec4& color);
	glm::vec4 GetColor() const;
private:
	static glm::vec2 applyOffSet(const glm::vec2& pos, const glm::vec2& scale, Anchor anchor);
	glm::vec2 m_virtualRes = {1920,1080};
	glm::vec2 m_pos = {0,0};
	glm::vec2 m_scale = {1,1};
	Anchor m_anchor = Anchor::Center;
	glm::vec4 m_color = {1.0f,1.0f,1.0,1.0f};
};
