#pragma once

#include "Animation.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace KGR
{
	namespace Animation
	{
		/** @brief runtime playback state for a skeletal animation instance. */
		class AnimationComponent
		{
		public:
			AnimationComponent() = default;
			~AnimationComponent() = default;

			/**
			 * @brief binds a skeleton and its clips to this component and activates the first valid clip.
			 * @param skeleton pointer to the joint hierarchy.
			 * @param clips available animation clips.
			 */
			void Init(const Skeleton* skeleton, const std::vector<AnimationClip>* clips);

			/**
			 * @brief switches to the clip at @p index and resets playback time.
			 * @param index clip index.
			 */
			void SetClip(size_t index);

			/**
			 * @brief advances the animation and recomputes all joint matrices.
			 * @param deltaTime seconds since the last frame.
			 */
			void Update(float deltaTime);

			/** @brief returns the skinning matrices indexed by joint ID. */
			const std::vector<glm::mat4>& GetLastBoneMatrices() const;

			/** @brief returns the number of clips available. */
			size_t GetClipCount() const;

		private:
			const Skeleton* m_skeleton = nullptr;
			const std::vector<AnimationClip>* m_clips = nullptr;
			const AnimationClip* m_clip = nullptr;

			size_t m_currentClipIdx = 0;
			float m_currentTime = 0.0f;

			std::vector<int> m_rootJointIds;
			std::unordered_map<int, const Joint*> m_jointById;
			std::unordered_map<int, const Track*> m_trackByJointId;

			std::vector<glm::mat4> m_globalMatrices;
			std::vector<glm::mat4> m_lastBoneMatrices;

			/** @brief rebuilds track map and clip pointer from the current clip index. */
			void RebuildClipData();

			/** @brief recursively computes the global transform for a joint and its descendants. */
			void CalculateBoneTransform(const Joint& joint, const glm::mat4& parentTransform);

			glm::vec3 InterpolatePosition(float time, const Track& track) const;
			glm::quat InterpolateRotation(float time, const Track& track) const;
			glm::vec3 InterpolateScale(float time, const Track& track) const;
		};

		template<typename KF, typename LerpFn>
		auto InterpolateKeyframes(float time, const std::vector<KF>& keys, LerpFn lerp)
			-> decltype(keys[0].m_value)
		{
			if (keys.size() == 1 || time <= keys.front().time)
				return keys.front().m_value;

			if (time >= keys.back().time)
				return keys.back().m_value;

			auto it = std::lower_bound(keys.begin(), keys.end(), time,
				[](const KF& k, float t) { return k.time < t; });

			const auto& k0 = *(it - 1);
			const auto& k1 = *it;
			const float f = glm::clamp((time - k0.time) / (k1.time - k0.time), 0.0f, 1.0f);
			return lerp(k0.m_value, k1.m_value, f);
		}
	}
}