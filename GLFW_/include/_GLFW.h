#pragma once
#include "glfw/glfw3.h"
#include "GLFW/glfw3native.h"
#include <glm/glm.hpp>
#include <ranges>
#include <thread>

namespace KGR
{
	namespace _GLFW
	{
		enum class WinState
		{
			Error,
			Windowed,
			FullScreen
		};

		struct WinInfo
		{
			glm::ivec2 m_pos = { 0,0 };
			bool m_posUpdated = false;

			glm::ivec2 m_size = { 0,0 };
			bool m_sizeUpdated = false;
			WinState State = WinState::Error;
		};

		enum class MonitorType
		{
			Primary,
			Current
		};

		struct Monitor
		{
			GLFWmonitor* glfwMonitor = nullptr;
		};

		struct Window
		{
			Window();

			static void Init();

			static void Destroy();
			static void PollEvent();
			static void AddHint(int hint, int value);

			const GLFWwindow& GetWindow() const;
			GLFWwindow& GetWindow();
			const GLFWwindow* GetWindowPtr() const;
			GLFWwindow* GetWindowPtr();

			void CreateMyWindow(glm::ivec2 size, const char* name, Monitor* monitor, Window* window);
			void DestroyMyWindow();
			bool ShouldClose() const;

			void SetSize(glm::ivec2 size);
			void SetPos(glm::ivec2 pos);
			void SetWindowState(WinState state, Monitor* monitor = nullptr);
			void UpdateParameters();

			glm::ivec2 GetPos() const;
			glm::ivec2 GetSize() const;

			bool PositionUpdated() const;
			bool SizeUpdated() const;

			template<WinState state>
			bool IsState() const
			{
				return m_info.State == state;
			}

			template<MonitorType state>
			Monitor GetMonitor() const
			{
				if constexpr (state == MonitorType::Primary)
					return { glfwGetPrimaryMonitor() };
				else
				{
					int monitorCount = 0;
					GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

					int finalIndex = 0;
					float minDistance = std::numeric_limits<float>::max();

					glm::vec2 windowMiddlePoint = m_info.m_pos + m_info.m_size / 2;

					for (int i = 0; i < monitorCount; ++i)
					{
						GLFWmonitor* current = monitors[i];
						glm::ivec2 monitorPos;
						glfwGetMonitorPos(current, &monitorPos.x, &monitorPos.y);
						glm::vec2 monitorFloatPosition = { monitorPos.x, monitorPos.y };
						const GLFWvidmode* videoMode = glfwGetVideoMode(current);
						glm::vec2 monitorSize = { videoMode->width, videoMode->height };
						glm::vec2 monitorMiddlePoint = monitorFloatPosition + monitorSize / 2.0f;

						// TODO length square ? dot product ? glm ?

						float distance = glm::length(windowMiddlePoint - monitorMiddlePoint);
						if (distance < minDistance)
						{
							minDistance = distance;
							finalIndex = i;
						}
					}

					return Monitor{ monitors[finalIndex] };
				}
			}

		private:

			static void PosCallBack(GLFWwindow* window, int posX, int posY);
			static void SizeCallBack(GLFWwindow* window, int width, int height);

			void Windowed();
			void FullScreen(Monitor monitor);

			GLFWwindow* m_window;
			WinInfo m_info;

			glm::ivec2 m_lasWindowedPos = { 0,0 };
			glm::ivec2 m_lasWindowedSize = { 0,0 };
		};
	}
}

