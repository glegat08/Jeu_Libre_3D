#include "Core/LightComponent.h"
#include "ts_ecs.h"
#include "Core/Window.h"

template<LightData::Type LightType>
void UpdateLightComponents(const std::unique_ptr<KGR::RenderWindow>& window, ts::Scene& scene)
{
	scene.Query<LightComponent<LightType>, TransformComponent>()
		.Each([&](ts::Entity e, LightComponent<LightType>& light, TransformComponent& transform) {window->RegisterLight(light, transform); });
}
