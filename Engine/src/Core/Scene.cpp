#include "Core/Scene.h"
#include "Core/Window.h"

Scene::Scene(const KGR::Tools::Chrono<float>::Time& time) : m_time(time)
{

}
KGR::Tools::Chrono<float>::Time Scene::GetTime() const
{
	return m_time;
}
