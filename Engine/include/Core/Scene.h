#pragma once
#include "Tools/Chrono.h"
class SceneManager;

struct Scene
{
	Scene(const KGR::Tools::Chrono<float>::Time& time);
	virtual void Init(SceneManager* manager) = 0;
	KGR::Tools::Chrono<float>::Time GetTime() const;
	virtual ~Scene() = default;
	virtual void Update(float dt) = 0;
	virtual void Render() = 0;
	
private:
	KGR::Tools::Chrono<float>::Time m_time;
};
