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

		struct Skeleton
		{
			std::string name;
			std::vector<Joint> m_joints;
		};

		struct VectorKeyFrame
		{
			float time;
			glm::vec3 m_value;
		};

		struct QuaternionKeyFrame
		{
			float time;
			glm::quat m_value;
		};

		struct Track
		{
			int nodeId;
			std::vector<VectorKeyFrame> m_positions;
			std::vector<QuaternionKeyFrame> m_rotations;
			std::vector<VectorKeyFrame> m_scales;
		};

		struct AnimationClip
		{
			std::string name;
			float duration = 0.0f;
			std::vector<Track> m_tracks;
		};
	}
}