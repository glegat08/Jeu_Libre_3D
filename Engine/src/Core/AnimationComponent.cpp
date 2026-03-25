#include "Core/AnimationComponent.h"

namespace KGR
{
	namespace Animation
	{
		void AnimationComponent::Init(const Skeleton* skeleton, const AnimationClip* clip)
		{
			m_skeleton = skeleton;
			m_clip = clip;
			m_currentTime = 0.0f;

			if (m_skeleton)
			{
				m_globalMAtrices.resize(m_skeleton->m_joints.size(), glm::mat4(1.0f));
				m_lastBoneMatrices.resize(m_skeleton->m_joints.size(), glm::mat4(1.0f));
			}
		}

		void AnimationComponent::Update(float deltaTime)
		{
			if (!m_skeleton || !m_clip)
				return;

			m_currentTime += deltaTime;
			if (m_currentTime > m_clip->duration)
				m_currentTime = fmod(m_currentTime, m_clip->duration);

			std::vector<bool> isChild(m_skeleton->m_joints.size(), false);
			for (const auto& joint : m_skeleton->m_joints)
			{
				for (int childId : joint.m_children)
					isChild[childId] = true;
			}

			for (const auto& joint : m_skeleton->m_joints)
			{
				if (!isChild[joint.id])
					CalculateBoneTransform(joint, glm::mat4(1.0f));
			}
		}

		const std::vector<glm::mat4>& AnimationComponent::GetLastBoneMatrices() const
		{
			return m_lastBoneMatrices;
		}

		void AnimationComponent::CalculateBoneTransform(const Joint& joint, const glm::mat4& parentTransform)
		{
			const Track* currentTrack = nullptr;
			if (m_clip)
			{
				for (const auto& track : m_clip->m_tracks)
				{
					if (track.nodeId == joint.id)
					{
						currentTrack = &track;
						break;
					}
				}
			}

			glm::vec3 translation = joint.translation;
			glm::quat rotation = joint.rotation;
			glm::vec3 scale = joint.scale;

			if (currentTrack)
			{
				if (!currentTrack->m_positions.empty())
					translation = InterpolatePosition(m_currentTime, *currentTrack);
				if (!currentTrack->m_rotations.empty())
					rotation = InterpolateRotation(m_currentTime, *currentTrack);
				if (!currentTrack->m_scales.empty())
					scale = InterpolateScale(m_currentTime, *currentTrack);
			}

			glm::mat4 matTranslation = glm::translate(glm::mat4(1.0f), translation);
			glm::mat4 matRotation = glm::mat4_cast(rotation);
			glm::mat4 matScale = glm::scale(glm::mat4(1.0f), scale);
			glm::mat4 localTransform = matTranslation * matRotation * matScale;

			glm::mat4 globalTransform = parentTransform * localTransform;
			m_globalMAtrices[joint.id] = globalTransform;
			m_lastBoneMatrices[joint.id] = globalTransform * joint.inverseBindMatrix;

			for (int childId : joint.m_children)
			{
				for (const auto& childJoint : m_skeleton->m_joints)
				{
					if (childJoint.id == childId)
					{
						CalculateBoneTransform(childJoint, globalTransform);
						break;
					}
				}
			}
		}

		glm::vec3 AnimationComponent::InterpolatePosition(float time, const Track& track)
		{
			if (track.m_positions.size() == 1) 
				return track.m_positions[0].m_value;

			for (size_t i = 0; i < track.m_positions.size() - 1; ++i) 
			{
				if (time < track.m_positions[i + 1].time) 
				{
					float factor = (time - track.m_positions[i].time) / (track.m_positions[i + 1].time - track.m_positions[i].time);
					return glm::mix(track.m_positions[i].m_value, track.m_positions[i + 1].m_value, factor);
				}
			}

			return track.m_positions.back().m_value;
		}

		glm::quat AnimationComponent::InterpolateRotation(float time, const Track& track)
		{
			if (track.m_rotations.size() == 1) 
				return track.m_rotations[0].m_value;

			for (size_t i = 0; i < track.m_rotations.size() - 1; ++i) 
			{
				if (time < track.m_rotations[i + 1].time) 
				{
					float factor = (time - track.m_rotations[i].time) / (track.m_rotations[i + 1].time - track.m_rotations[i].time);
					return glm::normalize(glm::slerp(track.m_rotations[i].m_value, track.m_rotations[i + 1].m_value, factor));
				}
			}

			return track.m_rotations.back().m_value;
		}

		glm::vec3 AnimationComponent::InterpolateScale(float time, const Track& track)
		{
			if (track.m_scales.size() == 1) 
				return track.m_scales[0].m_value;

			for (size_t i = 0; i < track.m_scales.size() - 1; ++i)
			{
				if (time < track.m_scales[i + 1].time)
				{
					float factor = (time - track.m_scales[i].time) / (track.m_scales[i + 1].time - track.m_scales[i].time);
					return glm::mix(track.m_scales[i].m_value, track.m_scales[i + 1].m_value, factor);
				}
			}
			return track.m_scales.back().m_value;
		}
	}
}