#pragma once

#include "Core/Mesh.h"
#include "Core/TrasformComponent.h"
#include "Core/Texture.h"
#include <string>

struct ObjectState
{
	MeshComponent      mesh;
	TransformComponent transform;
	TextureComponent   texture;

	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale    = { 1.0f, 1.0f, 1.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };

	std::string name      = "Object";
	std::string modelPath;
	std::string modelName;
	bool isAnimating	  = false;

	void ResetTransform()
	{
		position = { 0.0f, 0.0f, 0.0f };
		scale    = { 1.0f, 1.0f, 1.0f };
		rotation = { 0.0f, 0.0f, 0.0f };
	}

	void ApplyTransform()
	{
		transform.SetPosition(position);
		transform.SetScale(scale);
		transform.SetRotation(rotation);
	}
};
