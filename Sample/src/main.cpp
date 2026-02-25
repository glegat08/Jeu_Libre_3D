#include <filesystem>

#include "imgui.h"
#include "VulkanCore.h"
#include "_GLFW.h"
#include "Backends/imgui_impl_glfw.h"
#include "Backends/imgui_impl_vulkan.h"
#include "Core/ManagerImple.h"
#include "KGR_ImGui.h"


int main(int argc, char** argv)
{

	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();

	fileManager::SetGlobalFIlePath(projectRoot / "Ressources");
	STBManager::SetGlobalFIlePath(projectRoot / "Ressources");
	TOLManager::SetGlobalFIlePath(projectRoot / "Ressources");


	KGR::_GLFW::Window::Init();
	KGR::_GLFW::Window::AddHint(GLFW_CLIENT_API, GLFW_NO_API);
	KGR::_GLFW::Window::AddHint(GLFW_RESIZABLE, GLFW_TRUE);
	KGR::_GLFW::Window window;


	window.CreateMyWindow({ 1400, 900 }, "GC goes Vulkan", nullptr, nullptr);


	KGR::_Vulkan::VulkanCore app(&window.GetWindow());
	KGR::_ImGui::ImGuiCore imguiCore;

	app.initVulkan();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(1.5f);
	style.FontScaleDpi = 1.5f;

	imguiCore.InitImGui(&app, &window);

	bool show_demo_window = true;
	bool show_another_window = false;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	do
	{
		KGR::_GLFW::Window::PollEvent();

		imguiCore.BeginFrame(KGR::_ImGui::ContextTarget::Engine);

		if (show_demo_window)
			 ImGui::ShowDemoWindow(&show_demo_window);

		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");
			ImGui::Text("This is some useful text.");
			ImGui::Checkbox("Demo Window", &show_demo_window);
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
			ImGui::ColorEdit3("clear color", (float*)&clear_color);

			if (ImGui::Button("Button"))
				counter++;

			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;

			ImGui::End();
		}

		 ImGui::Render();
		 app.drawFrame(ImGui::GetDrawData());
	} 
	while (!window.ShouldClose());

	app.GetDevice().Get().waitIdle();

	imguiCore.Destroy();
	window.DestroyMyWindow();
    KGR::_GLFW::Window::Destroy();

}