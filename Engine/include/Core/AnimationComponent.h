#pragma once

#include "Animation.h"
#include <vector>

namespace KGR
{
	namespace Animation
	{
		class AnimationComponent
		{
		public:
			AnimationComponent() = default;
			~AnimationComponent() = default;

			void Init(const Skeleton* skeleton, const AnimationClip* clip);
			void Update(float deltaTime);

			const std::vector<glm::mat4>& GetLastBoneMatrices() const;

		private:
			const Skeleton* m_skeleton = nullptr;
			const AnimationClip* m_clip = nullptr;
			float m_currentTime = 0.0f;

			std::vector<glm::mat4> m_globalMAtrices;
			std::vector<glm::mat4> m_lastBoneMatrices;

			void CalculateBoneTransform(const Joint& joint, const glm::mat4& parentTransform);

			glm::vec3 InterpolatePosition(float time, const Track& track);
			glm::quat InterpolateRotation(float time, const Track& track);
			glm::vec3 InterpolateScale(float time, const Track& track);
		};
	}
}