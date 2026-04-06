#include "Core/AnimationComponent.h"
#include <algorithm>
#include <cmath>

namespace KGR
{
	namespace Animation
	{
		void AnimationComponent::Init(const Skeleton* skeleton, const std::vector<AnimationClip>* clips)
		{
			m_skeleton = skeleton;
			m_clips = clips;
			m_currentTime = 0.0f;
			m_currentClipIdx = 0;

			m_rootJointIds.clear();
			m_jointById.clear();

			if (!m_skeleton)
				return;

			const size_t count = m_skeleton->m_joints.size();
			m_globalMatrices.assign(count, glm::mat4(1.0f));
			m_lastBoneMatrices.assign(count, glm::mat4(1.0f));

			// Single pass: build the joint map and collect every child ID.
			// Anything not in childIds is a root.
			std::unordered_set<int> childIds;
			for (const auto& joint : m_skeleton->m_joints)
			{
				m_jointById[joint.id] = &joint;
				childIds.insert(joint.m_children.begin(), joint.m_children.end());
			}

			for (const auto& joint : m_skeleton->m_joints)
				if (!childIds.count(joint.id))
					m_rootJointIds.push_back(joint.id);

			if (!m_clips)
				return;

			// Activate the first clip that actually has data.
			auto it = std::find_if(m_clips->begin(), m_clips->end(),
				[](const AnimationClip& c) 
				{ 
					return !c.m_tracks.empty() && c.duration > 0.0f;
				});

			m_currentClipIdx = (it != m_clips->end())
				? static_cast<size_t>(std::distance(m_clips->begin(), it)) : 0;

			RebuildClipData();
		}

		void AnimationComponent::SetClip(size_t index)
		{
			if (!m_clips || index >= m_clips->size())
				return;

			const auto& clip = (*m_clips)[index];
			if (clip.m_tracks.empty() || clip.duration <= 0.0f)
				return;

			m_currentClipIdx = index;
			m_currentTime = 0.0f;
			RebuildClipData();
		}

		void AnimationComponent::RebuildClipData()
		{
			m_trackByJointId.clear();
			m_clip = nullptr;

			if (!m_clips || m_currentClipIdx >= m_clips->size())
				return;

			m_clip = &(*m_clips)[m_currentClipIdx];
			for (const auto& track : m_clip->m_tracks)
				m_trackByJointId[track.nodeId] = &track;
		}

		void AnimationComponent::Update(float deltaTime)
		{
			if (!m_skeleton || !m_clip)
				return;

			m_currentTime = fmod(m_currentTime + deltaTime, m_clip->duration);

			for (int rootId : m_rootJointIds)
			{
				auto it = m_jointById.find(rootId);
				if (it != m_jointById.end())
					CalculateBoneTransform(*it->second, glm::mat4(1.0f));
			}
		}

		const std::vector<glm::mat4>& AnimationComponent::GetLastBoneMatrices() const
		{
			return m_lastBoneMatrices;
		}

		size_t AnimationComponent::GetClipCount() const
		{
			return m_clips ? m_clips->size() : 0;
		}

		void AnimationComponent::CalculateBoneTransform(const Joint& joint, const glm::mat4& parentTransform)
		{
			auto pos = joint.translation;
			auto rot = joint.rotation;
			auto scl = joint.scale;

			auto it = m_trackByJointId.find(joint.id);
			if (it != m_trackByJointId.end())
			{
				const Track& track = *it->second;
				if (!track.m_positions.empty()) 
					pos = InterpolatePosition(m_currentTime, track);

				if (!track.m_rotations.empty()) 
					rot = InterpolateRotation(m_currentTime, track);

				if (!track.m_scales.empty())   
					scl = InterpolateScale(m_currentTime, track);
			}

			const glm::mat4 local = glm::translate(glm::mat4(1.0f), pos)
				* glm::mat4_cast(rot)
				* glm::scale(glm::mat4(1.0f), scl);

			const glm::mat4 global = parentTransform * local;
			m_globalMatrices[joint.id] = global;
			m_lastBoneMatrices[joint.id] = global * joint.inverseBindMatrix;

			for (int childId : joint.m_children)
			{
				auto childIt = m_jointById.find(childId);
				if (childIt != m_jointById.end())
					CalculateBoneTransform(*childIt->second, global);
			}
		}

		glm::vec3 AnimationComponent::InterpolatePosition(float time, const Track& track) const
		{
			return InterpolateKeyframes(time, track.m_positions,
				[](const glm::vec3& a, const glm::vec3& b, float t) 
				{
					return glm::mix(a, b, t);
				});
		}

		glm::quat AnimationComponent::InterpolateRotation(float time, const Track& track) const
		{
			return InterpolateKeyframes(time, track.m_rotations,
				[](const glm::quat& a, const glm::quat& b, float t)
				{
					return glm::normalize(glm::slerp(a, b, t));
				});
		}

		glm::vec3 AnimationComponent::InterpolateScale(float time, const Track& track) const
		{
			return InterpolateKeyframes(time, track.m_scales,
				[](const glm::vec3& a, const glm::vec3& b, float t) 
				{
					return glm::mix(a, b, t);
				});
		}
	}
}