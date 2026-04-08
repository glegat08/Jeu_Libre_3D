#pragma once

#include <string>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace KGR
{
	namespace Animation
	{
		/** @brief joint in a skeleton. */
		struct Joint
		{
			std::string name;
			int id;
			glm::mat4 inverseBindMatrix{ 1.0f };
			glm::vec3 translation{ 0.0f };
			glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
			glm::vec3 scale{ 1.0f };
			std::vector<int> m_children;
		};

		/** @brief hierarchical collection of joints. */
		struct Skeleton
		{
			std::string name;
			std::vector<Joint> m_joints;
		};

		/** @brief vec3 keyframe for position or scale channels. */
		struct VectorKeyFrame
		{
			float time;
			glm::vec3 m_value;
		};

		/** @brief quaternion keyframe for rotation channels. */
		struct QuaternionKeyFrame
		{
			float time;
			glm::quat m_value;
		};

		/** @brief animation keyframes for a single joint. */
		struct Track
		{
			int nodeId;
			std::vector<VectorKeyFrame> m_positions;
			std::vector<QuaternionKeyFrame> m_rotations;
			std::vector<VectorKeyFrame> m_scales;
		};

		/** @brief named animation sequence with duration and per-joint tracks. */
		struct AnimationClip
		{
			std::string name;
			float duration = 0.0f;
			std::vector<Track> m_tracks;
		};
	}
}