#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Tools/Util.h"
#include <concepts>

struct Rep
{
	static constexpr glm::vec3 up = {0,1,0};
	static constexpr glm::vec3 right = {1,0,0};
	static constexpr glm::vec3 forward = {0,0,-1};
};

template<typename Type>
concept IsValidRep = requires
{
	{ Type::up }-> std::convertible_to <const  glm::vec3&>;
	{ Type::right }-> std::convertible_to < const  glm::vec3&>;
	{ Type::forward }-> std::convertible_to < const  glm::vec3&>;
};

struct RotData
{
	enum class Dir
	{
		Up,
		Down,
		Forward,
		Backward,
		Left,
		Right
	};
	template<Dir dir , IsValidRep rep = Rep>
	static glm::vec3 ToVector()
	{
		if	constexpr (dir == Dir::Up)
			return glm::normalize(rep::up);
		else if constexpr (dir == Dir::Down)
			return glm::normalize(-1 * rep::up);
		else if constexpr (dir == Dir::Forward)
			return glm::normalize(rep::forward);
		else if constexpr (dir == Dir::Backward)
			return glm::normalize(-1 * rep::forward);
		else if constexpr (dir == Dir::Right)
			return glm::normalize(rep::right);
		else 
			return glm::normalize(-1 * rep::right);
	}

	enum class Orientation
	{
		Yaw,
		Pitch,
		Roll
	};
	template<Orientation orientation, IsValidRep rep = Rep>
	static glm::vec3 ToAxes()
	{
		if	constexpr (orientation == Orientation::Pitch)
			return glm::normalize(rep::right);
		else if constexpr (orientation == Orientation::Yaw)
			return glm::normalize(rep::up);
		else 
			return glm::normalize(rep::forward);
	}
};




class TransformComponent
{
public:
	// POS
	glm::vec3 GetPosition() const;
	void SetPosition(const glm::vec3& other);
	void Translate(const glm::vec3& other);
	glm::mat4 GetTranslationMatrix();

	// SCALE
	glm::vec3 GetScale() const;
	void SetScale(const glm::vec3& other);
	glm::mat4 GetScaleMatrix();

	// ORIENTATION
	glm::quat GetOrientation() const;
	void SetOrientation(const glm::quat& other);
	template<RotData::Orientation orientation, IsValidRep rep = Rep>
	void RotateQuat(float angleRad);
	template<IsValidRep rep = Rep>
	void LookAt(const glm::vec3& target);
	template<IsValidRep rep = Rep>
	void LookAtDir(const glm::vec3& target);

	// ROTATION 
	glm::vec3 GetRotation() const;
	void SetRotation(const glm::vec3& other);
	template<RotData::Orientation orientation, IsValidRep rep = Rep>
	void RotateEuler(float angleRad);

	glm::mat4 GetRotationMatrix();

	// AXES
	template<RotData::Dir dir, IsValidRep rep = Rep>
	glm::vec3 GetLocalAxe() const;

	// FULL TRANSFORM
	glm::mat4 GetFullTransform();
	bool IsDirty() const;
private:
	void UpdateEulerAngle();
	void UpdateQuaternion();
	
	DataDirty<glm::quat> m_rotation = {true,glm::quat{1,0,0,0}};
	 glm::mat4 m_rotationMat = glm::mat4{1.0f};
	 glm::vec3 m_eulerAngle = glm::vec3{0,0,0};
	DataDirty<glm::vec3> m_position = { true,glm::vec3{0,0,0} };
	 glm::mat4 m_positionMat = glm::mat4{ 1.0f };
	DataDirty <glm::vec3> m_scale = { true,glm::vec3{1,1,1} };
	 glm::mat4 m_scaleMat = glm::mat4{ 1.0f };
	 glm::mat4 m_fullTransform = glm::mat4{ 1.0f };
};

template <RotData::Orientation orientation, IsValidRep rep>
void TransformComponent::RotateQuat(float angleRad)
{
	m_rotation.data = glm::normalize(glm::rotate(m_rotation.data, angleRad, RotData::ToAxes<orientation, rep>()));
	UpdateEulerAngle();
	m_rotation.isDirty = true;
}

template <IsValidRep rep>
void TransformComponent::LookAt(const glm::vec3& target)
{
	glm::vec3 forward = glm::normalize(target - m_position.data);
	m_rotation.data = glm::quatLookAt(forward, RotData::ToVector<RotData::Dir::Up, rep>());
	UpdateEulerAngle();
	m_rotation.isDirty = true;

}

template <IsValidRep rep>
void TransformComponent::LookAtDir(const glm::vec3& target)
{
	glm::vec3 forward = glm::normalize(target - glm::vec3{0,0,0});
	m_rotation.data = glm::quatLookAt(forward, RotData::ToVector<RotData::Dir::Up, rep>());
	UpdateEulerAngle();
	m_rotation.isDirty = true;
}

template <RotData::Orientation orientation, IsValidRep rep>
void TransformComponent::RotateEuler(float angleRad)
{

	m_eulerAngle += angleRad * RotData::ToAxes <orientation,rep>();
	UpdateQuaternion();
	m_rotation.isDirty = true;

}

template <RotData::Dir dir, IsValidRep rep>
glm::vec3 TransformComponent::GetLocalAxe() const
{
	glm::vec3 result = m_rotation.data * RotData::ToVector<dir, rep>();
	return result;

}
