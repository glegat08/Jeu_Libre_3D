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
		/**
		 * @brief Single joint in a skeleton — name, ID, inverse bind matrix, local TRS, and child IDs.
		 */
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

		/**
		 * @brief Hierarchical collection of joints used for skeletal animation.
		 */
		struct Skeleton
		{
			std::string name;
			std::vector<Joint> m_joints;
		};

		/**
		 * @brief Keyframe for vec3 channels (position or scale).
		 */
		struct VectorKeyFrame
		{
			float time;
			glm::vec3 m_value;
		};

		/**
		 * @brief Keyframe for rotation channels.
		 */
		struct QuaternionKeyFrame
		{
			float time;
			glm::quat m_value;
		};

		/**
		 * @brief Animation data for one joint — position, rotation, and scale keyframes over time.
		 */
		struct Track
		{
			int nodeId;
			std::vector<VectorKeyFrame> m_positions;
			std::vector<QuaternionKeyFrame> m_rotations;
			std::vector<VectorKeyFrame> m_scales;
		};

		/**
		 * @brief Named collection of tracks that defines one animation sequence and its duration.
		 */
		struct AnimationClip
		{
			std::string name;
			float duration = 0.0f;
			std::vector<Track> m_tracks;
		};
	}
}