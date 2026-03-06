#pragma once

#include "Core/Mesh.h"
#include "Core/TrasformComponent.h"
#include "Core/Texture.h"
#include <string>

/// @brief Holds all runtime data for a single scene object: geometry, transform, texture, and metadata.
///
/// position, scale, and rotation are the "staging" values edited by the UI.
/// Call ApplyTransform() to push them into the TransformComponent.
struct ObjectState
{
	MeshComponent      mesh;       ///< GPU mesh data.
	TransformComponent transform;  ///< Component used by the render pipeline.
	TextureComponent   texture;    ///< Diffuse texture data.

	glm::vec3 position = { 0.0f, 0.0f, 0.0f }; ///< World-space position (staging).
	glm::vec3 scale	   = { 1.0f, 1.0f, 1.0f }; ///< Non-uniform scale (staging).
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f }; ///< Euler angles in radians: pitch/yaw/roll (staging).

	std::string name = "Object"; ///< Display name shown in the editor.
	std::string modelPath;       ///< Absolute path to the source OBJ file.
	std::string modelName;       ///< Filename extracted from modelPath, used as a label.
	bool isAnimating = false;    ///< Whether the RMF animation is currently playing.

	/// @brief Resets position, scale, and rotation to their default values.
	void ResetTransform()
	{
		position = { 0.0f, 0.0f, 0.0f };
		scale = { 1.0f, 1.0f, 1.0f };
		rotation = { 0.0f, 0.0f, 0.0f };
	}

	/// @brief Pushes the staging position, scale, and rotation into the TransformComponent.
	void ApplyTransform()
	{
		transform.SetPosition(position);
		transform.SetScale(scale);
		transform.SetRotation(rotation);
	}
};