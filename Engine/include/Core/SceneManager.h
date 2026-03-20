#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <optional>

#include "Tools/Chrono.h"
class Scene;


class SceneManager
{
public:
	SceneManager() = default;
	virtual ~SceneManager() = default;
	void AddScene(std::unique_ptr<Scene> scene,const std::string& name, bool isActive = false);
	void Run(const KGR::Tools::Chrono<float>::Time& fixedTime);
	void SetCurrentScene(const std::string& name);
	virtual bool LoopCondition()const = 0;
private:
	Scene* GetCurrentScene();
	std::optional<std::uint64_t> m_currentIndex;
	std::unordered_map<std::uint64_t,std::unique_ptr<Scene>> m_scenes;
};