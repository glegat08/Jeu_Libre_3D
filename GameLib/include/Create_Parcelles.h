#ifndef CREATE_PARCELLES_H
#define CREATE_PARCELLES_H

#include <glm/vec3.hpp>

#include "Core/GLBLoader.h"
#include "Core/GLBEntityFactory.h"
#include "ts_ecs.h"
#include "Core/Window.h"

#include "Components.h"

void Create_Parcelles(const std::unique_ptr<KGR::RenderWindow>& window, ts::Scene& scene, KGR::GLB::GLBCache& glbCache, KGR::GLB::GLBNeutralTextures& neutral,
	const std::string& meshPath, const std::string& texturePath, glm::vec3 pos)
{
	ts::Entity parcelle;

	const KGR::GLB::GLBAsset* ParcelleAsset = glbCache.Get(meshPath, window->App());
	Texture& texture = TextureLoader::Load(texturePath, window->App());
	if (ParcelleAsset)
		parcelle = KGR::GLB::CreateGLBEntity<ts::Scene>
		(
			scene, *ParcelleAsset,
			pos, glm::vec3{ 30.0f,0.0f,0.0f }, glm::vec3(1.0f),
			neutral, KGR::GLB::GLBSkinOverride{ .baseColor = &texture }
		).entity;

	

	scene.Add<ParcelleComponent>(parcelle, ParcelleComponent{});
	scene.Add<HealtComponent>(parcelle, HealtComponent{ 20 });
}

#endif
