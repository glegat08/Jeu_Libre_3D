#include "Core/UiComponent.h"

#include <stdexcept>

glm::vec2 UiComponent::VrToNdc(const glm::vec2& vec, const glm::vec2& vr, float aspectRatio, bool scale)
{
	return { VrToNdcX(vec.x,vr.x,aspectRatio,scale),VrToNdcY(vec.y,vr.y,scale) };
}

glm::vec2 UiComponent::NdcToVr(const glm::vec2& vec, const glm::vec2& vr, float aspectRatio, bool scale)
{
	return  { NdcToVrX(vec.x,vr.x,aspectRatio,scale),NdcToVrY(vec.y,vr.y,scale) };
}

float UiComponent::VrToNdcX(float x, float vrX, float aspectRatio, bool scale)
{
	return (x / vrX) * (2.0f * aspectRatio) - (scale == false ? aspectRatio : 0.0f);
}

float UiComponent::VrToNdcY(float y, float vrY, bool scale)
{
	return (y / vrY) * 2.0f - (scale == false ? 1.0f : 0.0f);

}

float UiComponent::NdcToVrX(float x, float vrX, float aspectRatio, bool scale)
{
	return vrX * ((x + (scale == false ? aspectRatio : 0.0f)) / (2.0f * aspectRatio));
}

float UiComponent::NdcToVrY(float y, float vrY, bool scale)
{
	return vrY * ((y + (scale == false ? 1.0f : 0.0f)) / 2.0f);
}

UiComponent::UiComponent(const glm::vec2& vr,Anchor anchor) : m_virtualRes(vr),m_anchor(anchor)
{
}

void UiComponent::SetVr(const glm::vec2& vr)
{
	m_virtualRes = vr;
}

glm::vec2 UiComponent::GetVr() const
{
	return m_virtualRes;
}


void UiComponent::SetPos(const glm::vec2& pos)
{
	m_pos = pos;
}

glm::vec2 UiComponent::GetPosVr() const
{
	return m_pos;
}

glm::vec2 UiComponent::GetPosNdc(float aspectRatio) const
{
	auto pos = UiComponent::VrToNdc(m_pos, m_virtualRes, aspectRatio, false);
	return  UiComponent::applyOffSet(pos,GetScaleNdc(aspectRatio),m_anchor);
}

void UiComponent::SetScale(const glm::vec2& scale)
{
	m_scale = scale;
}

glm::vec2 UiComponent::GetScaleVr() const
{
	return m_scale;
}

glm::vec2 UiComponent::GetScaleNdc(float aspectRatio) const
{
	
	return UiComponent::VrToNdc(m_scale, m_virtualRes, aspectRatio, true);
}

void UiComponent::SetAnchor(Anchor anchor)
{
	m_anchor = anchor;
}

UiComponent::Anchor UiComponent::GetAnchor() const
{
	return m_anchor;
}

void UiComponent::SetColor(const glm::vec4& color)
{
	m_color = color;
}

glm::vec4 UiComponent::GetColor() const
{
	return m_color;
}

glm::vec2 UiComponent::applyOffSet(const glm::vec2& pos, const glm::vec2& scale, Anchor anchor)
{
	switch (anchor)
	{
	case Anchor::Center:
		return pos;
	case  Anchor::LeftTop:
		return { pos.x + (scale.x / 2.0f),pos.y + (scale.y / 2.0f) };
	case Anchor::RightTop:
		return { pos.x - (scale.x / 2.0f),pos.y + (scale.y / 2.0f) };
	case  Anchor::LeftBottom:
		return { pos.x + (scale.x / 2.0f),pos.y - (scale.y / 2.0f) };
	case Anchor::RightBottom:
		return { pos.x - (scale.x / 2.0f),pos.y -   (scale.y / 2.0f) };
	default:
		throw std::out_of_range("not handle");
	}
}
 
